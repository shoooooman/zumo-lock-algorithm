var:
    path <- {0}: rqs the robot will move on
    rqs  <- {0}: rqs the robot is reqesting for locks
    locks <- {0}: rqs the robot locks
    lrd <- 0: last request date
    nrs <- [0, ..., 0]: number of requests for each cell
    reach <- true: if a robot have reached its destinaton
    cp <- (0, 0): current position of the robot

P1: get paths, get locks, move, release
const:
    MAX_LOCKS: the number of locks robots can request once

var:
    rd <- inf: the rest of distance from the robot to its destination
    waiting_from <- {0}: robots from which the robot is waiting for reply
    num_of_get_locks <- 0: number of locks the robot got
    clock <- 0: logical clock

while (true)
    wait (reach = true);
    dest <- get_dest();
    path <- get_path(cp, dest);
    reach <- false;
    while (path != {0})
        rqs <- rqs and path[0 .. MAX_LOCKS-1-num_of_get_locks];
        path <- path \ rqs;
        for each cell in rqs
            nrs[cell] <- nrc[cell] + 1;
        lrd <- clock + 1;
        for each j in {1, ..., n} \ {i}
            send REQUEST(i, lrd, rqs, cp, dest, nrs) to p_j;
        wait (waiting_from  = {0}); // wait until get a reply
        locks <- locks and rqs[0 .. num_of_get_locks-1];
        for each cell in rqs[0 .. num_of_get_locks-1]
            nrs[cell] <- 0;
        rqs <- rqs \ {rqs[0 .. num_of_get_locks-1]};
        send MOVE(locks) to p_i;

when REPLY(j, ok) is received do
    num_of_get_locks <- min(num_of_get_locks, ok); // get min of all other nodes' accepted
    waiting_from <- waiting_from \ {j};


P2: reply other robots request
const:
    NUM_ROBOTS: the number of robots in this system
var:
    accepted <- 0: the number of cells the robot can accepte locks
    prio_i: priority of this robot

when REQUEST(j, k, rc, pos, d, cnt) is received do
    accepted <- 0;
    for each c in rc
        if      (c in locks)
            prio_i <- true;
        else if (c in rqs)
            if      (lrd < k)
                prio_i <- true;
            else if (lrd = k)
                if      (<cp, c> < <pos, c>) // <a, b> represent the distance between a and b
                    prio_i <- true;
                else if (nrs[c] > cnt[c])
                    prio_i <- true;
                else if (<c, dest> > <c, d>)
                    prio_i <- true;
                else if (i < j)
                    prio_i <- true;
                else
                    prio_i <- false;
            else
                prio_i <- false;
        else
            prio_i <- false;

        if (prio_i)
            accepted <- accepted + 1;
        else
            send REPLY(i, accepted) to p_j;
            break;
    send REPLY(i, accepted) to p_j;


P3: release locks

when passed cell c do
    locks <- locks \ {c}; // decremental release
    cp <- get_current_pos();
    if (cp = dest) reach <- true;
