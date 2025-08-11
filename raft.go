package raft

//
// this is an outline of the API that raft must expose to
// the service (or tester). see comments below for
// each of these functions for more details.
//
// rf = Make(...)
//   create a new Raft server.
// rf.Start(command interface{}) (index, term, isleader)
//   start agreement on a new log entry
// rf.GetState() (term, isLeader)
//   ask a Raft for its current term, and whether it thinks it is leader
// ApplyMsg
//   each time a new entry is committed to the log, each Raft peer
//   should send an ApplyMsg to the service (or tester)
//   in the same server.
//

import (
	"bytes"
	"math/rand"
	"sync"
	"sync/atomic"
	"time"

	"6.5840/labgob"
	"6.5840/labrpc"
)

// as each Raft peer becomes aware that successive log entries are
// committed, the peer should send an ApplyMsg to the service (or
// tester) on the same server, via the applyCh passed to Make(). set
// CommandValid to true to indicate that the ApplyMsg contains a newly
// committed log entry.
//
// in part 3D you'll want to send other kinds of messages (e.g.,
// snapshots) on the applyCh, but set CommandValid to false for these
// other uses.
type ApplyMsg struct {
	CommandValid bool
	Command      interface{}
	CommandIndex int
	CommandTerm  int

	// For 3D:
	SnapshotValid bool
	Snapshot      []byte
	SnapshotTerm  int
	SnapshotIndex int
}

type LogEntry struct {
	Term    int
	Command interface{}
}

const (
	Follower = iota
	Candidate
	Leader
)

// A Go object implementing a single Raft peer.
type Raft struct {
	mu        sync.Mutex
	peers     []*labrpc.ClientEnd
	persister *Persister
	me        int
	dead      int32

	currentTerm int
	votedFor    int
	log         []LogEntry

	commitIndex int
	lastApplied int

	nextIndex  []int
	matchIndex []int

	lastHeartbeatTime int64
	isLeader          bool
	state             int
	voteCount         int
	applyCh           chan ApplyMsg

	snapshotIndex int
	snapshotTerm  int
	snapshot      []byte
}

// return currentTerm and whether this server
// believes it is the leader.
func (rf *Raft) GetState() (int, bool) {
	// Your code here (3A).
	rf.mu.Lock()
	defer rf.mu.Unlock()
	return int(rf.currentTerm), rf.isLeader
}

// save Raft's persistent state to stable storage,
// where it can later be retrieved after a crash and restart.
// see paper's Figure 2 for a description of what should be persistent.
// before you've implemented snapshots, you should pass nil as the
// second argument to persister.Save().
// after you've implemented snapshots, pass the current snapshot
// (or nil if there's not yet a snapshot).
func (rf *Raft) persist() {
	// Your code here (3C).
	// Example:
	// w := new(bytes.Buffer)
	// e := labgob.NewEncoder(w)
	// e.Encode(rf.xxx)
	// e.Encode(rf.yyy)
	// raftstate := w.Bytes()
	// rf.persister.Save(raftstate, nil)
	w := new(bytes.Buffer)
	e := labgob.NewEncoder(w)
	e.Encode(rf.currentTerm)
	e.Encode(rf.votedFor)
	e.Encode(rf.log)
	e.Encode(rf.snapshotIndex)
	e.Encode(rf.snapshotTerm)
	data := w.Bytes()
	rf.persister.Save(data, rf.snapshot)

}

// restore previously persisted state.
func (rf *Raft) readPersist(data []byte) {
	if data == nil || len(data) < 1 { // bootstrap without any state?
		rf.log = append(rf.log, LogEntry{Term: 0, Command: nil})
		rf.snapshotIndex = 0
		rf.snapshotTerm = 0
		return
	}
	// Your code here (3C).
	// Example:
	// r := bytes.NewBuffer(data)
	// d := labgob.NewDecoder(r)
	// var xxx
	// var yyy
	// if d.Decode(&xxx) != nil ||
	//    d.Decode(&yyy) != nil {
	//   error...
	// } else {
	//   rf.xxx = xxx
	//   rf.yyy = yyy
	// }
	if data == nil || len(data) < 1 {
		return
	}
	r := bytes.NewBuffer(data)
	d := labgob.NewDecoder(r)
	var currentTerm, votedFor, snapshotIndex, snapshotTerm int
	var log []LogEntry
	if d.Decode(&currentTerm) != nil ||
		d.Decode(&votedFor) != nil ||
		d.Decode(&log) != nil ||
		d.Decode(&snapshotIndex) != nil ||
		d.Decode(&snapshotTerm) != nil {
		return
	}
	rf.currentTerm = currentTerm
	rf.votedFor = votedFor
	rf.log = log
	rf.snapshotIndex = snapshotIndex
	rf.snapshotTerm = snapshotTerm
	rf.snapshot = rf.persister.ReadSnapshot()
	rf.lastApplied = rf.snapshotIndex
	// commitIndex也应至少为快照索引
	rf.commitIndex = max(rf.commitIndex, rf.snapshotIndex)
}

// the service says it has created a snapshot that has
// all info up to and including index. this means the
// service no longer needs the log through (and including)
// that index. Raft should now trim its log as much as possible.
func (rf *Raft) Snapshot(index int, snapshot []byte) {
	// Your code here (3D).
	rf.mu.Lock()
	defer rf.mu.Unlock()
	if index <= rf.snapshotIndex {
		return
	}
	// 计算物理下标
	offset := index - rf.snapshotIndex
	if offset < 0 || offset >= len(rf.log) {
		// 非法快照请求，直接返回
		return
	}
	rf.snapshotTerm = rf.log[offset].Term
	// 截断日志
	// 快照后，保留一条 entry 作为 dummy
	rf.log = append([]LogEntry{{Term: rf.snapshotTerm, Command: nil}}, rf.log[offset+1:]...)
	rf.snapshotIndex = index
	// 持久化
	rf.snapshot = snapshot
	rf.persistAndSaveSnapshot(snapshot)

}

func (rf *Raft) persistAndSaveSnapshot(snapshot []byte) {
	w := new(bytes.Buffer)
	e := labgob.NewEncoder(w)
	e.Encode(rf.currentTerm)
	e.Encode(rf.votedFor)
	e.Encode(rf.log)
	e.Encode(rf.snapshotIndex)
	e.Encode(rf.snapshotTerm)
	data := w.Bytes()
	rf.persister.Save(data, snapshot)
}

// example RequestVote RPC arguments structure.
// field names must start with capital letters!
type RequestVoteArgs struct {
	Term         int
	CandidateId  int
	LastLogIndex int
	LastLogTerm  int
}
type RequestVoteReply struct {
	Term        int
	VoteGranted bool
}

// example RequestVote RPC handler.
func (rf *Raft) RequestVote(args *RequestVoteArgs, reply *RequestVoteReply) {
	// Your code here (3A, 3B).
	rf.mu.Lock()
	defer rf.mu.Unlock()
	if args.Term < rf.currentTerm {
		reply.Term = rf.currentTerm
		reply.VoteGranted = false
		return
	}
	if args.Term > rf.currentTerm {
		rf.currentTerm = args.Term
		rf.state = Follower
		rf.votedFor = -1
		rf.isLeader = false
	}
	reply.Term = rf.currentTerm
	// 没投过票或投给自己，且日志最新
	upToDate := false
	lastLogIndex := rf.snapshotIndex + len(rf.log) - 1
	lastLogTerm := rf.log[len(rf.log)-1].Term // 最后一条日志（含dummy后的有效条目）

	if args.LastLogTerm > lastLogTerm ||
		(args.LastLogTerm == lastLogTerm && args.LastLogIndex >= (lastLogIndex)) {
		upToDate = true
	}
	if (rf.votedFor == -1 || rf.votedFor == args.CandidateId) && upToDate {
		rf.votedFor = args.CandidateId
		rf.lastHeartbeatTime = time.Now().UnixNano()
		reply.VoteGranted = true
		rf.persist()
	} else {
		reply.VoteGranted = false
	}

}

// example code to send a RequestVote RPC to a server.
// server is the index of the target server in rf.peers[].
// expects RPC arguments in args.
// fills in *reply with RPC reply, so caller should
// pass &reply.
// the types of the args and reply passed to Call() must be
// the same as the types of the arguments declared in the
// handler function (including whether they are pointers).
//
// The labrpc package simulates a lossy network, in which servers
// may be unreachable, and in which requests and replies may be lost.
// Call() sends a request and waits for a reply. If a reply arrives
// within a timeout interval, Call() returns true; otherwise
// Call() returns false. Thus Call() may not return for a while.
// A false return can be caused by a dead server, a live server that
// can't be reached, a lost request, or a lost reply.
//
// Call() is guaranteed to return (perhaps after a delay) *except* if the
// handler function on the server side does not return.  Thus there
// is no need to implement your own timeouts around Call().
//
// look at the comments in ../labrpc/labrpc.go for more details.
//
// if you're having trouble getting RPC to work, check that you've
// capitalized all field names in structs passed over RPC, and
// that the caller passes the address of the reply struct with &, not
// the struct itself.
func (rf *Raft) sendRequestVote(server int, args *RequestVoteArgs, reply *RequestVoteReply) bool {
	ok := rf.peers[server].Call("Raft.RequestVote", args, reply)
	return ok
}

type AppendEntriesArgs struct {
	Term         int
	LeaderId     int
	PrevLogIndex int
	PrevLogTerm  int
	Entries      []LogEntry
	LeaderCommit int
}
type AppendEntriesReply struct {
	Term          int
	Success       bool
	ConflictIndex int // follower期望的nextIndex
	ConflictTerm  int // follower冲突的term
}

func min(a, b int) int {
	if a < b {
		return a
	}
	return b
}

func max(a, b int) int {
	if a < b {
		return b
	}
	return a
}

func (rf *Raft) AppendEntries(args *AppendEntriesArgs, reply *AppendEntriesReply) {
	rf.mu.Lock()
	defer rf.mu.Unlock()

	// 规则 1: 如果 Leader 的任期 (args.Term) 小于当前节点的任期，直接拒绝。
	if args.Term < rf.currentTerm {
		reply.Term = rf.currentTerm
		reply.Success = false
		return
	}

	// 如果收到来自更高任期的 Leader 的 RPC，无论如何都要更新自己的状态。
	if args.Term > rf.currentTerm {
		rf.currentTerm = args.Term
		rf.state = Follower
		rf.votedFor = -1
		// isLeader 状态的转变在 ticker 和选举逻辑中处理，这里确保 votedFor 和 term 正确
		rf.persist() // 任期和投票信息是持久化状态
	}

	rf.state = Follower // 收到合法 Leader 的心跳，即使是 Candidate 也要变回 Follower
	rf.isLeader = false

	// 重置选举计时器，因为我们收到了一个合法的 Leader 的心跳或日志。
	rf.lastHeartbeatTime = time.Now().UnixNano()
	reply.Term = rf.currentTerm

	// --- 日志一致性检查 ---
	lastLogIndex := rf.snapshotIndex + len(rf.log) - 1

	// 规则 2: 如果 Leader 发来的 PrevLogIndex 超出了本地日志的范围，说明本地日志太短。
	if args.PrevLogIndex > lastLogIndex {
		reply.Success = false
		// 告诉 Leader 下次应该从哪里开始发，即本地日志的末尾+1 (绝对索引)。
		reply.ConflictIndex = lastLogIndex + 1
		reply.ConflictTerm = -1 // 无法提供冲突任期号
		return
	}

	if args.PrevLogIndex < rf.snapshotIndex {
		// Leader 发来的日志已经被我们快照了，这是一种异常情况，
		// 但我们仍需正确处理。拒绝并让 Leader 发快照。
		reply.Success = false
		reply.ConflictIndex = rf.snapshotIndex + 1
		reply.ConflictTerm = -1
		return
	}

	var termAtPrevLogIndex int
	if args.PrevLogIndex == rf.snapshotIndex {
		termAtPrevLogIndex = rf.snapshotTerm
	} else {
		termAtPrevLogIndex = rf.log[args.PrevLogIndex-rf.snapshotIndex].Term
	}

	if termAtPrevLogIndex != args.PrevLogTerm {
		reply.Success = false
		reply.ConflictTerm = termAtPrevLogIndex
		// 为了让 Leader 快速回退，找到本地日志中这个冲突任期的第一个条目。
		conflictIndex := args.PrevLogIndex
		relativeIndex := conflictIndex - rf.snapshotIndex
		for relativeIndex > 0 && rf.log[relativeIndex-1].Term == reply.ConflictTerm {
			relativeIndex--
		}
		reply.ConflictIndex = relativeIndex + rf.snapshotIndex
		return
	}

	// --- 日志追加与截断 ---
	// 规则 3 & 4: 如果一致性检查通过，就查找冲突点，截断并追加新日志。
	for i, entry := range args.Entries {
		index := args.PrevLogIndex + 1 + i
		if index > lastLogIndex {
			rf.log = append(rf.log, args.Entries[i:]...)
			rf.persist()
			break
		}
		if rf.log[index-rf.snapshotIndex].Term != entry.Term {
			rf.log = rf.log[:index-rf.snapshotIndex]
			rf.log = append(rf.log, args.Entries[i:]...)
			rf.persist()
			break
		}
	}

	// --- 更新 Commit Index ---
	// 规则 5: 如果 Leader 的 commitIndex 大于本地的 commitIndex，更新本地 commitIndex。
	if args.LeaderCommit > rf.commitIndex {
		lastNewEntryIndex := args.PrevLogIndex + len(args.Entries)
		rf.commitIndex = min(args.LeaderCommit, lastNewEntryIndex)
		rf.applyLog()
	}

	reply.Success = true
}

func (rf *Raft) sendAppendEntries(server int, args *AppendEntriesArgs, reply *AppendEntriesReply) bool {
	return rf.peers[server].Call("Raft.AppendEntries", args, reply)
}

type InstallSnapshotArgs struct {
	Term              int
	LeaderId          int
	LastIncludedIndex int
	LastIncludedTerm  int
	Data              []byte
}

type InstallSnapshotReply struct {
	Term int
}

func (rf *Raft) InstallSnapshot(args *InstallSnapshotArgs, reply *InstallSnapshotReply) {
	rf.mu.Lock()

	reply.Term = rf.currentTerm

	if args.Term < rf.currentTerm {
		rf.mu.Unlock()
		return
	}
	if args.Term > rf.currentTerm {
		rf.currentTerm = args.Term
		rf.state = Follower
		rf.votedFor = -1
		rf.isLeader = false
		rf.persist()
	}
	rf.lastHeartbeatTime = time.Now().UnixNano()

	if args.LastIncludedIndex <= rf.snapshotIndex {
		// 已有快照更新
		rf.mu.Unlock()
		return
	}

	newLog := []LogEntry{{Term: args.LastIncludedTerm, Command: nil}}
	minPhysIndex := args.LastIncludedIndex - rf.snapshotIndex
	if minPhysIndex+1 < len(rf.log) {
		newLog = append(newLog, rf.log[minPhysIndex+1:]...)
	}
	rf.log = newLog

	rf.snapshotIndex = args.LastIncludedIndex
	rf.snapshotTerm = args.LastIncludedTerm
	rf.lastApplied = args.LastIncludedIndex // 快照索引就是最后应用的索引
	rf.commitIndex = max(rf.commitIndex, rf.snapshotIndex)
	rf.snapshot = args.Data
	rf.persistAndSaveSnapshot(args.Data)

	// 通知上层应用安装快照，使用同步方式但释放锁
	msg := ApplyMsg{
		CommandValid:  false,
		SnapshotValid: true,
		Snapshot:      args.Data,
		SnapshotTerm:  args.LastIncludedTerm,
		SnapshotIndex: args.LastIncludedIndex,
	}

	rf.mu.Unlock()
	rf.applyCh <- msg
}

// the service using Raft (e.g. a k/v server) wants to start
// agreement on the next command to be appended to Raft's log. if this
// server isn't the leader, returns false. otherwise start the
// agreement and return immediately. there is no guarantee that this
// command will ever be committed to the Raft log, since the leader
// may fail or lose an election. even if the Raft instance has been killed,
// this function should return gracefully.
//
// the first return value is the index that the command will appear at
// if it's ever committed. the second return value is the current
// term. the third return value is true if this server believes it is
// the leader.
func (rf *Raft) Start(command interface{}) (int, int, bool) {
	rf.mu.Lock()
	defer rf.mu.Unlock()
	if rf.state != Leader {
		return -1, int(rf.currentTerm), false
	}
	index := rf.snapshotIndex + len(rf.log)
	entry := LogEntry{
		Term:    int(rf.currentTerm),
		Command: command,
	}
	rf.log = append(rf.log, entry)
	rf.persist()
	go rf.replicateLog()
	return index, int(rf.currentTerm), true
}

func (rf *Raft) replicateLog() {
	rf.mu.Lock()
	term := rf.currentTerm
	rf.mu.Unlock()
	for i := range rf.peers {
		if i == rf.me {
			continue
		}
		go func(server int) {
			rf.mu.Lock()
			if rf.state != Leader || rf.currentTerm != term {
				rf.mu.Unlock()
				return
			}

			if rf.nextIndex[server] <= rf.snapshotIndex {
				// 发送快照
				args := InstallSnapshotArgs{
					Term:              rf.currentTerm,
					LeaderId:          rf.me,
					LastIncludedIndex: rf.snapshotIndex,
					LastIncludedTerm:  rf.snapshotTerm,
					Data:              rf.snapshot,
				}
				rf.mu.Unlock()

				reply := InstallSnapshotReply{}
				ok := rf.peers[server].Call("Raft.InstallSnapshot", &args, &reply)
				if !ok {
					return
				}

				rf.mu.Lock()
				defer rf.mu.Unlock()
				if reply.Term > rf.currentTerm {
					rf.currentTerm = reply.Term
					rf.state = Follower
					rf.votedFor = -1
					rf.isLeader = false
					rf.persist()
					return
				}
				rf.nextIndex[server] = rf.snapshotIndex + 1
				rf.matchIndex[server] = rf.snapshotIndex
				return
			}

			// 需要发送的日志区间
			nextIdx := rf.nextIndex[server]
			prevLogIndex := nextIdx - 1
			prevLogTerm := 0
			if prevLogIndex == rf.snapshotIndex {
				prevLogTerm = rf.snapshotTerm
			} else if prevLogIndex > rf.snapshotIndex {
				prevLogTerm = rf.log[prevLogIndex-rf.snapshotIndex].Term
			}

			var entries []LogEntry
			if nextIdx <= rf.snapshotIndex+len(rf.log)-1 {
				entries = make([]LogEntry, len(rf.log[nextIdx-rf.snapshotIndex:]))
				copy(entries, rf.log[nextIdx-rf.snapshotIndex:])
			}

			args := AppendEntriesArgs{
				Term:         int(term),
				LeaderId:     rf.me,
				PrevLogIndex: prevLogIndex,
				PrevLogTerm:  prevLogTerm,
				Entries:      entries,
				LeaderCommit: rf.commitIndex,
			}
			rf.mu.Unlock()

			reply := AppendEntriesReply{}
			ok := rf.sendAppendEntries(server, &args, &reply)
			if !ok {
				return
			}

			rf.mu.Lock()
			defer rf.mu.Unlock()
			if reply.Term > rf.currentTerm {
				rf.currentTerm = reply.Term
				rf.state = Follower
				rf.votedFor = -1
				rf.isLeader = false
				rf.persist()
				return
			}

			if reply.Success {
				rf.matchIndex[server] = prevLogIndex + len(entries)
				rf.nextIndex[server] = rf.matchIndex[server] + 1
				rf.updateCommitIndex()
			} else {
				// 冲突优化
				if reply.ConflictTerm != -1 {
					found := false
					lastPhysIdx := -1
					for i := len(rf.log) - 1; i >= 0; i-- {
						if rf.log[i].Term == reply.ConflictTerm {
							lastPhysIdx = i
							found = true
							break
						}
					}
					if found {
						rf.nextIndex[server] = rf.snapshotIndex + lastPhysIdx + 1
					} else {
						rf.nextIndex[server] = reply.ConflictIndex
					}
				} else {
					rf.nextIndex[server] = reply.ConflictIndex
				}
			}
		}(i)
	}
}

func (rf *Raft) updateCommitIndex() {
	maxPossibleIndex := rf.snapshotIndex + len(rf.log) - 1 // 正确计算最大实际索引

	for N := maxPossibleIndex; N > rf.commitIndex; N-- {
		if N <= rf.snapshotIndex {
			continue
		}
		logIdx := N - rf.snapshotIndex

		// Ensure we don't go out of bounds
		if logIdx < 0 || logIdx >= len(rf.log) {
			continue
		}
		count := 1 // 包括自己
		for i := range rf.peers {
			if i != rf.me && rf.matchIndex[i] >= N {
				count++
			}
		}
		if count > len(rf.peers)/2 && rf.log[logIdx].Term == int(rf.currentTerm) {
			rf.commitIndex = N
			rf.applyLog()
			break
		}
	}
}

func (rf *Raft) applyLog() {
	// 逐个应用日志，确保顺序性
	for rf.lastApplied < rf.commitIndex {
		nextIndex := rf.lastApplied + 1
		idx := nextIndex - rf.snapshotIndex
		if idx <= 0 || idx >= len(rf.log) {
			break
		}

		rf.lastApplied = nextIndex
		msg := ApplyMsg{
			CommandValid: true,
			Command:      rf.log[idx].Command,
			CommandIndex: int(rf.lastApplied),
			CommandTerm:  rf.log[idx].Term,
		}

		// 临时释放锁发送消息，确保顺序
		rf.mu.Unlock()
		rf.applyCh <- msg
		rf.mu.Lock()
	}
} // the tester doesn't halt goroutines created by Raft after each test,
// but it does call the Kill() method. your code can use killed() to
// check whether Kill() has been called. the use of atomic avoids the
// need for a lock.
//
// the issue is that long-running goroutines use memory and may chew
// up CPU time, perhaps causing later tests to fail and generating
// confusing debug output. any goroutine with a long-running loop
// should call killed() to check whether it should stop.
func (rf *Raft) Kill() {
	atomic.StoreInt32(&rf.dead, 1)
	// Your code here, if desired.
}

func (rf *Raft) killed() bool {
	z := atomic.LoadInt32(&rf.dead)
	return z == 1
}

func randElectionTimeout() time.Duration {
	return time.Duration(300+rand.Intn(200)) * time.Millisecond
}

func (rf *Raft) ticker() {
	for !rf.killed() {
		time.Sleep(10 * time.Millisecond)
		rf.mu.Lock()
		timeout := randElectionTimeout()
		isLeader := rf.isLeader
		lastHeartbeat := rf.lastHeartbeatTime
		rf.mu.Unlock()

		if !isLeader && time.Since(time.Unix(0, lastHeartbeat)) > timeout {
			rf.startElection()
		}
	}
}

func (rf *Raft) startElection() {
	rf.mu.Lock()
	rf.state = Candidate
	rf.currentTerm++
	term := rf.currentTerm
	rf.votedFor = rf.me
	rf.voteCount = 1
	rf.persist()

	lastLogIndex := rf.snapshotIndex + len(rf.log) - 1
	lastLogTerm := rf.log[len(rf.log)-1].Term

	args := RequestVoteArgs{
		Term:         term,
		CandidateId:  rf.me,
		LastLogIndex: lastLogIndex,
		LastLogTerm:  lastLogTerm,
	}
	rf.mu.Unlock()

	for i := range rf.peers {
		if i == rf.me {
			continue
		}
		go func(server int) {
			reply := RequestVoteReply{}
			if rf.sendRequestVote(server, &args, &reply) {
				rf.mu.Lock()
				defer rf.mu.Unlock()
				if rf.state != Candidate || reply.Term < rf.currentTerm {
					return
				}
				if reply.Term > rf.currentTerm {
					rf.currentTerm = reply.Term
					rf.state = Follower
					rf.votedFor = -1
					rf.isLeader = false
					rf.persist()
					return
				}
				if reply.VoteGranted {
					rf.voteCount++
					if rf.voteCount > len(rf.peers)/2 && rf.state == Candidate {
						rf.state = Leader
						rf.isLeader = true
						rf.lastHeartbeatTime = time.Now().UnixNano()
						// 初始化 nextIndex, matchIndex
						for i := range rf.peers {
							rf.nextIndex[i] = rf.snapshotIndex + len(rf.log)
							rf.matchIndex[i] = rf.snapshotIndex
						}
						go rf.sendHeartbeats()
					}
				}
			}
		}(i)
	}
}

func (rf *Raft) sendHeartbeats() {
	for !rf.killed() {
		rf.mu.Lock()
		if rf.state != Leader {
			rf.mu.Unlock()
			return
		}
		rf.mu.Unlock()
		rf.replicateLog()
		time.Sleep(100 * time.Millisecond)
	}
}

// the service or tester wants to create a Raft server. the ports
// of all the Raft servers (including this one) are in peers[]. this
// server's port is peers[me]. all the servers' peers[] arrays
// have the same order. persister is a place for this server to
// save its persistent state, and also initially holds the most
// recent saved state, if any. applyCh is a channel on which the
// tester or service expects Raft to send ApplyMsg messages.
// Make() must return quickly, so it should start goroutines
// for any long-running work.
func Make(peers []*labrpc.ClientEnd, me int,
	persister *Persister, applyCh chan ApplyMsg) *Raft {
	rf := &Raft{}
	rf.peers = peers
	rf.persister = persister
	rf.me = me

	// 不要初始化 log, snapshotIndex, snapshotTerm
	rf.votedFor = -1
	rf.currentTerm = 0
	rf.commitIndex = 0
	rf.lastApplied = 0
	rf.nextIndex = make([]int, len(peers))
	rf.matchIndex = make([]int, len(peers))
	rf.lastHeartbeatTime = time.Now().UnixNano()
	rf.isLeader = false
	rf.state = Follower
	rf.applyCh = applyCh
	rf.voteCount = 0

	rf.snapshot = rf.persister.ReadSnapshot()

	// initialize from state persisted before a crash
	rf.readPersist(persister.ReadRaftState())

	// 如果 log 为空，初始化为 dummy entry
	if len(rf.log) == 0 {
		rf.log = append(rf.log, LogEntry{Term: rf.snapshotTerm, Command: nil})
	}

	go rf.ticker()
	return rf
}

