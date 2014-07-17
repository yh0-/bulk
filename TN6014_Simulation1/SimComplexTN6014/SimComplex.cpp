/**  Simulating complex network system.
 *
 *  Network system diagram:
 *                                              _
 *                                    _ _ _ _  | |
 *    Px ---->                 --->  |_|_|_|_| |_|
 *  Time:5s   \  _ _ _ _     /                 S1, Time:4s
 *              |_|_|_|_| (x)                   _
 *            /            R \        _ _ _ _  | |
 *    Py ---->        Time:1s  --->  |_|_|_|_| |_|
 *  Time:10s                                   S2, Time:7s
 *
 *  The performance metrics have to be calculated are:
 *  a. Mean interarrival time for Px and Py.
 *  b. Mean service time for R (Router).
 *  c. Mean service time for S1 (Server 1)
 *  d. Mean service time for S2 (Server 2)
 *  e. Total Px served by S1, total Py served by S2.
 *
 *  The simulation program runs until (200 s simulation time OR
 *  Px served equal 100 packets).
 *
 *  Program created by:
 *  1. Mohd Azi Bin Abdullah
 *  2. Yahya Sjahrony
 *
 *  Adapted from the following resources:
 *  1. Lecturer slide 2: Simulation Concepts and Components.
 *  2. Lecturer slide 3: Inside Simulation Software.
 *  3. Discrete-Event System Simulation - Jerry Banks (Chapter 4).
 *  4. Event-Driven Simulation example program from Apache
 *     C++ Standard Library User's Guide.
 *  5. Single-server queueing system, C++ version of mm1.c in
 *     Law--Kelton, 2000. Oct 2002.
 *  6. CSC 270 simulation example, adapted March 1996 by J. Clarke from
 *     Turing original by M. Molle.
 *
 */

//---------------------------------------------------------------------------
// System includes
//---------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue> //priority_queue
#include <cmath> //log, sqrt, cos, sin
#include <cstdlib> //random, srandom
#include <ctime> //time
#include <unistd.h> //getopt
#include <sstream> //istringstream

//XXX
#if defined(_WIN32) || defined(_WIN64)
    #define random rand
    #define srandom srand
#endif

//---------------------------------------------------------------------------
// Standard names
//---------------------------------------------------------------------------

using std::cout;
using std::ostream;
using std::ofstream;
using std::queue;
using std::priority_queue;
using std::vector;
using std::string;
using std::istringstream;

//---------------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------------

// Define the B-events (unconditional events) of the system:
#define B1 1 // Event: B1, Type: arrival, Change in system state:
             // Px arrives and enters router queue.
#define B2 2 // Event: B2, Type: arrival, Change in system state:
             // Py arrives and enters router queue.
#define B3 3 // Event: B3, Type: Departure, Change in system state:
             // Router completes work and outputs
             // Px to S1 queue, Py to S2 queue.
#define B4 4 // Event: B4, Type: Departure, Change in system state:
             // S1 completes work and packet leave the network
             // (increment packets served Px by 1).
#define B5 5 // Event: B5, Type: Departure, Change in system state:
             // S2 completes work and packet leave the network
             // (increment packets served Py by 1).

// Router/Server states
#define IDLE 0
#define BUSY 1

// Network system diagram
const string DIAGRAM =
"\n"
"=========================================================\n"
"    Complex Network System Simulation         _ \n"
"                                    _ _ _ _  | | \n"
"    Px ---->                 --->  |_|_|_|_| |_| \n"
"  Time:5s   \\  _ _ _ _     /                 S1, Time:4s \n"
"              |_|_|_|_| (x)                   _ \n"
"            /            R \\        _ _ _ _  | | \n"
"    Py ---->        Time:1s  --->  |_|_|_|_| |_| \n"
"  Time:10s                                   S2, Time:7s\n"
"========================================================="
"\n";

//---------------------------------------------------------------------------
// Global variables
//---------------------------------------------------------------------------

// Command line options
int dFlag = 0; // debug flag
int nFlag = 0; // random number stream generation flag
int tFlag = 0; // simulation ending time flag
int xFlag = 0; // simulation ending packet count flag

double gSimulationTime; // Current simulation time
double gLastEventTime; // Time of last event before the current one

// Given mean interarrival times of Px and Py respectively
double gPxInterArrivalTime, gPyInterArrivalTime;
// Given mean service times of Router, Server 1 and Server 2 respectively
double gRouterServiceTime, gS1ServiceTime, gS2ServiceTime;
double gSigma, gSigma1, gSigma2; // Standard deviation of service times

// Stream for output files for debugging
ofstream gOutfile1;
ofstream gOutfile2;

//---------------------------------------------------------------------------
// Helper to write to standard output and file
//---------------------------------------------------------------------------

class Tee {
    ostream &first, &second;
    template<typename T> friend Tee& operator<< (Tee&, T);
public:
    Tee (ostream& f, ostream& s) : first(f), second(s) {}
} gTee (cout, gOutfile1);

template <typename T>
Tee& operator<< (Tee& rt, T val) {
    rt.first << val;
    rt.second << val;
    return rt;
}

//---------------------------------------------------------------------------
// Pseudo random number generator
//---------------------------------------------------------------------------

class RandomStream {
public:
    //static const int RANGE = 0x7fffffff; //RAND_MAX
    static const double PI = 3.1415927;
    double numNormals;
    double saveNormal;
    RandomStream ();
    ~RandomStream () {}
    double Uniform ();
    double Exponential (double);
    double Normal (double, double);
    double NextNormal (double, double);
};

RandomStream* gpRandomStream;

// Constructor
RandomStream::RandomStream () {
    numNormals = 0;
    saveNormal = 0;
}

// Return the next random number.
double RandomStream::Uniform () {
    return (random()/(double)RAND_MAX);
}

// Return the next exponentially distributed random number
double RandomStream::Exponential (double mean) {
    return -mean*log(Uniform());
}

// Return the next normally distributed random number generator.
double RandomStream::Normal (double mean, double sigma) {
    double retNormal;
    if (numNormals == 0) {
        double r1 = Uniform();
        double r2 = Uniform();
        retNormal = sqrt(-2*log(r1))*cos(2*PI*r2);
        saveNormal = sqrt(-2*log(r1))*sin(2*PI*r2);
        numNormals = 1;
    } else {
        numNormals = 0;
        retNormal = saveNormal;
    }
    return retNormal*sigma + mean;
}

// Return a normally distributed random number generator with positive value.
double RandomStream::NextNormal (double mean, double sigma) {
    double ret;
    while( (ret = Normal(mean, sigma)) < 0 );
    return ret;
}

//---------------------------------------------------------------------------
// Event & Event Handler
//---------------------------------------------------------------------------

class Handler;

// Event class
class Event {
public:
    Handler* handler_; // handler to call when event ready
    double time_; // time at which event is ready
    int type_; // event type
    Event (int type, double time) : time_(time), type_(type) {}
};

// Event Handler base class
class Handler {
public:
    virtual ~Handler () {} // virtual method
    virtual void handle (Event*) = 0; // pure virtual method
};

//---------------------------------------------------------------------------
// Event Scheduler
//---------------------------------------------------------------------------

// Compare two events based on their time.
struct EventCompare {
    bool operator () (const Event* left, const Event* right) const {
        return left->time_ > right->time_;
    }
};

// Event scheduler class
class Scheduler {
public:
    Scheduler () : mEventQueue () {}
    ~Scheduler ();
    void Schedule (Handler*, Event*);
    Event* Deque ();
    void Dispatch (Event*);

private:
    priority_queue<Event*, vector<Event*>, EventCompare> mEventQueue;
    // The queue holds the future event list (FEL). The lower time has
    // higher priority

} gScheduler;

// Destructor
Scheduler::~Scheduler () {
    while (!mEventQueue.empty()) {
        delete mEventQueue.top ();
        mEventQueue.pop ();
    }
}

// Schedule event (insert to FEL)
void Scheduler::Schedule (Handler* h, Event* p) {
    if (dFlag) {
        gTee << "[" << "B" << p->type_ << " " << p->time_ << "] ";
    }
    p->handler_ = h;
    mEventQueue.push(p); // The queue is sorted automatically as
                         // new events are added.
}

// Return the next event (removes from FEL)
Event* Scheduler::Deque () {
    Event* p = NULL;
    if (!mEventQueue.empty()) {
        p = mEventQueue.top();
        mEventQueue.pop();
    }
    return p;
}

// Execute an event.
void Scheduler::Dispatch (Event* p) {
    p->handler_->handle(p); // calls the handler associated with the event
}

//---------------------------------------------------------------------------
// Packet & PacketQueue
//---------------------------------------------------------------------------

// Packet class for PacketQueue element
class Packet: public Event {
public:
    Packet (int type, double time): Event(type, time) {}
};
//XXX:

// Packet queue class
class PacketQueue {
private:
    queue<Packet*> mQueue;
    int mQueueSize;
    double mTotalEmptyQueueTime;

public:
    PacketQueue () : mQueueSize(0), mTotalEmptyQueueTime(0.0) {}
    ~PacketQueue ();
    void Enqueue (Packet*);
    Packet* Dequeue ();
    int QueueSize () const;
    double TotalEmptyQueueTime () const;
};

PacketQueue* gpRouterQueue,* gpS1Queue,* gpS2Queue;

// Destructor
PacketQueue::~PacketQueue () {
    while (mQueue.size() > 0) {
        delete mQueue.front ();
        mQueue.pop ();
    }
}

// Insert a new packet to the packet queue.
void PacketQueue::Enqueue (Packet* p) {
    if (mQueue.size() == 0) {
        mTotalEmptyQueueTime += gSimulationTime;
    }
    mQueue.push(p);
    mQueueSize += 1;
}

// Return the next packet (removes from the queue)
Packet* PacketQueue::Dequeue () {
    Packet* p = NULL;
    if (mQueue.size()) {
        p = mQueue.front();
        mQueue.pop();
        mQueueSize -= 1;
        if (mQueue.size() == 0) {
            mTotalEmptyQueueTime -= gSimulationTime;
        }
    }
    return p;
}

// Return packet queue size.
int PacketQueue::QueueSize () const {
    return mQueueSize;
}

// Return packet queue total empty time.
double PacketQueue::TotalEmptyQueueTime () const {
    double total;
    if (mQueueSize > 0) {
        total = mTotalEmptyQueueTime;
    } else {
        total = mTotalEmptyQueueTime + gSimulationTime;
    }
    return total;
}

//---------------------------------------------------------------------------
// Service entity
//---------------------------------------------------------------------------

class Service {
private:
    int mState;
    size_t mTotalPacket;
    double mTotalServiceTime;
    double mArrivalTime;
    double mTimeServiceBegin;
    double mTimeServiceEnd;
    double mTimePktWaitsInQueue;
    double mTimePktSpendsInSystem;
    double mIdleTimeOfService;
    double mServiceTime;

public:
    Service ();
    ~Service () {}
    int State () const { return mState; }
    void State (int state) { mState= state; }
    size_t TotalPacket () const { return mTotalPacket; }
    double TotalServiceTime () const { return mTotalServiceTime; }
    double ServiceTime() const { return mServiceTime; }
    void stats (double);
};

Service* gpRouter,* gpS1,* gpS2;

// Constructor
Service::Service () {
    mState = IDLE;
    mTotalServiceTime = 0.0;
    mTotalPacket = 0;
    mArrivalTime = 0.0;
    mTimeServiceBegin = 0.0;
    mTimeServiceEnd = 0.0;
    mTimePktWaitsInQueue = 0.0;
    mTimePktSpendsInSystem = 0.0;
    mIdleTimeOfService = 0.0;
    mServiceTime = 0.0;
}

// Service entity stats
void Service::stats (double t) {
    mArrivalTime = t;
    if (mArrivalTime > mTimeServiceEnd) {
        mTimeServiceBegin = mArrivalTime;
        mTimePktWaitsInQueue = 0;
        mIdleTimeOfService = mArrivalTime - mTimeServiceEnd;
    } else {
        mTimeServiceBegin = mTimeServiceEnd;
        mTimePktWaitsInQueue = mTimeServiceEnd - mArrivalTime;
        mIdleTimeOfService = 0;
    }

    mTimeServiceEnd = gSimulationTime;
    mServiceTime = mTimeServiceEnd - mTimeServiceBegin;
    mTimePktSpendsInSystem = mServiceTime + mTimePktWaitsInQueue;

    mTotalServiceTime += mServiceTime;
    mTotalPacket++;

    if (dFlag > 1) {
        gTee << "ArrivalTime=" << mArrivalTime << " "
             << "TimeServiceBegin=" << mTimeServiceBegin << " "
             << "ServiceTime=" << mServiceTime << " "
             << "TimeServiceEnd=" << mTimeServiceEnd << " "
             << "TimePktWaitsInQueue=" << mTimePktWaitsInQueue << " "
             << "TimePktSpendsInSystem=" << mTimePktSpendsInSystem << " "
             << "IdleTimeOfService=" << mIdleTimeOfService << " "
        ;
    }

    gOutfile2 << mArrivalTime << ","
              << mTimeServiceBegin << ","
              << mServiceTime << ","
              << mTimeServiceEnd << ","
              << mTimePktWaitsInQueue << ","
              << mTimePktSpendsInSystem << ","
              << mIdleTimeOfService << " "
    ;
}

//---------------------------------------------------------------------------
// Helper for collecting and reporting statistics
//---------------------------------------------------------------------------

class Stats {
private:
    size_t mTotalArrivals;
    // Total Px served by R so far
    size_t mTotalPxOnR;
    // Total Py served by R so far
    size_t mTotalPyOnR;
    double mTotalWaitingTime;
    // Mean interarrival times
    double mMeanPxInterArrivalTime, mMeanPyInterArrivalTime;
    // Mean service times
    double mRMeanServiceTime, mS1MeanServiceTime, mS2MeanServiceTime;
public:
    Stats ();
    ~Stats () {}
    void IncrementArrivals () { mTotalArrivals += 1; };
    size_t TotalArrivals () const { return mTotalArrivals; }
    void ComputeTotalWaitingTime (double interval) {
        mTotalWaitingTime += interval;
    };
    double TotalWaitingTime () const { return mTotalWaitingTime; }
    void ComputeMeanPxInterArrivalTime (double);
    void ComputeMeanPyInterArrivalTime (double);
    void Report ();
};

Stats* gpStats;

// Constructor
Stats::Stats () {
    mTotalArrivals = 0;
    mTotalPxOnR=0;
    mTotalPyOnR=0;
    mTotalWaitingTime = 0;
    mMeanPxInterArrivalTime = 0.0;
    mMeanPyInterArrivalTime = 0.0;
    mRMeanServiceTime = 0.0;
    mS1MeanServiceTime = 0.0;
    mS2MeanServiceTime = 0.0;
}

// Compute the mean interarrival times of Px so far
void Stats::ComputeMeanPxInterArrivalTime (double total) {
    mTotalPxOnR++;
    mMeanPxInterArrivalTime = total/mTotalPxOnR;
}

// Compute the mean interarrival times of Py so far
void Stats::ComputeMeanPyInterArrivalTime (double total) {
    mTotalPyOnR++;
    mMeanPyInterArrivalTime = total/mTotalPyOnR;
}

//---------------------------------------------------------------------------
// Departure Event Handler
//---------------------------------------------------------------------------

class DepartureHandler : public Handler {
public:
    ~DepartureHandler ();
    void handle (Event*);
} gDepartureHandler;

// Destructor
DepartureHandler::~DepartureHandler () {
}

// Event handler implementation for the derived class
void DepartureHandler::handle (Event* event) {
    if (dFlag > 1) {
        gTee << "\nDEBUG: DepartureHandler: ";
    }

    if (event->type_ == B3) {
        Packet* finished = gpRouterQueue->Dequeue();

        gpStats -> ComputeTotalWaitingTime (
                     gSimulationTime - finished->time_);

        // Check to see whether the router queue is empty
        if (gpRouterQueue->QueueSize () > 0) {
            if (dFlag > 1) {
                gTee << "{STATE: R BUSY} ";
            }

            // Schedule the next departure (B3 event).
            double serviceTime;
            !nFlag
                ? serviceTime = gpRandomStream->NextNormal(
                                                  gRouterServiceTime, gSigma)
                : serviceTime = gRouterServiceTime;

            gScheduler.Schedule(&gDepartureHandler,
                               new Event(B3, gSimulationTime + serviceTime));

        } else {
            gpRouter->State(IDLE); // Begin idle time
            if (dFlag > 1) {
                gTee << "{STATE: R IDLE} ";
            }
        }

        // Compute and display some stats on router so far
        if (dFlag > 1) {
            gTee << "SimulationTime=" << gSimulationTime << " "
                 << "LastEventTime=" << gLastEventTime << " "
                 << "finished=" << finished->time_
                 << " (B" << finished->type_ << ") "
            ;
        }

        gpStats->IncrementArrivals();

        gOutfile2 << "\nR" << "," << gpStats->TotalArrivals() << ",";

        if (finished->type_ == B1) {
            gOutfile2 << "Px" << ",";
        } else if (finished->type_ == B2) {
            gOutfile2 << "Py" << ",";
        }

        gpRouter->stats(finished->time_);

        if (finished->type_ == B1) {
            gpStats->ComputeMeanPxInterArrivalTime(finished->time_);

        } else if (finished->type_ == B2) {
            gpStats->ComputeMeanPyInterArrivalTime(finished->time_);
        }

        gOutfile2 << " ";

        if (finished->type_ == B1) {
            // Router completes work and outputs Px to S1 queue
            finished->time_ = gSimulationTime;
            gpS1Queue->Enqueue(finished);

            // C-event (conditional event): (C2)
            if (gpS1->State() == IDLE) {
                // Condition: packet in S1 queue and S1 is idle
                // Change in system state: S1 takes packet from its queue
                // and starts work.

                if (dFlag > 1) {
                    gTee << "{S1 starts work} ";
                }

                // Schedule the next departure (B4 event).
                double serviceTime;
                !nFlag
                    ? serviceTime = gpRandomStream->NextNormal(
                                                     gS1ServiceTime, gSigma1)
                    : serviceTime = gS1ServiceTime;

                gScheduler.Schedule(&gDepartureHandler,
                               new Event(B4, gSimulationTime + serviceTime));
                gpS1->State(BUSY);
            }
        } else if (finished->type_ == B2) {
            // Router completes work and outputs Py to S2 queue
            finished->time_ = gSimulationTime;
            gpS2Queue->Enqueue(finished);

            // C-event (conditional event): (C3)
            if ( gpS2->State() == IDLE) {
                // Condition: packet in S2 queue and S2 is idle
                // Change in system state: S2 takes packet from its queue
                // and starts work.

                if (dFlag > 1) {
                    gTee << "{S2 starts work} ";
                }

                // Schedule the next departure (B5 event).
                double serviceTime;
                !nFlag
                    ? serviceTime = gpRandomStream->NextNormal(
                                                     gS2ServiceTime, gSigma2)
                    : serviceTime = gS2ServiceTime;

                gScheduler.Schedule(&gDepartureHandler,
                               new Event(B5, gSimulationTime + serviceTime));

                gpS2->State(BUSY);
            }
        }

    } else if (event->type_ == B4) {
        Packet* finished = gpS1Queue->Dequeue();

        // Check to see whether S1 queue is empty
        if ( gpS1Queue->QueueSize () > 0) {
            if (dFlag > 1) {
                gTee <<"{STATE: S1 BUSY} ";
            }

            // Schedule the next departure (B4 event).
            double serviceTime;
            !nFlag
                ? serviceTime = gpRandomStream->NextNormal(
                                                     gS1ServiceTime, gSigma1)
                : serviceTime = gS1ServiceTime;

            gScheduler.Schedule(&gDepartureHandler,
                               new Event(B4, gSimulationTime + serviceTime));

        } else {
            gpS1->State(IDLE); // Begin idle time
            if (dFlag > 1) {
                gTee << "{STATE: S1 IDLE} ";
            }
        }

        // Compute and display some stats on S1 so far
        if (dFlag > 1) {
            gTee << "SimulationTime=" << gSimulationTime << " "
                 << "LastEventTime=" << gLastEventTime << " ";
        }

        gOutfile2 << "S1" << ","
                  << gpStats->TotalArrivals() << ","
                  << "Px" << ",";

        gpS1->stats(finished->time_);
        gOutfile2 << " ";

        delete finished;

    } else if (event->type_ == B5) {
        Packet* finished = gpS2Queue->Dequeue();

        // Check to see whether S2 queue is empty
        if ( gpS2Queue->QueueSize () > 0) {
            if (dFlag > 1) {
                gTee <<"{STATE: S2 BUSY} ";
            }

            // Schedule the next departure (B5 event).
            double serviceTime;
            !nFlag
                ? serviceTime = gpRandomStream->NextNormal(
                                                     gS2ServiceTime, gSigma2)
                : serviceTime = gS2ServiceTime;

            gScheduler.Schedule(&gDepartureHandler,
                               new Event(B5, gSimulationTime + serviceTime));

        } else {
            gpS2->State(IDLE); // Begin idle time
            if (dFlag > 1) {
                gTee << "{STATE: S2 IDLE} ";
            }
        }

        // Compute and display some stats on S2 so far
        if (dFlag > 1) {
            gTee << "SimulationTime=" << gSimulationTime << " "
                 << "LastEventTime=" << gLastEventTime << " ";
        }

        gOutfile2 << "S2" << ","
                  << gpStats->TotalArrivals() << ","
                  << "Py" << ",";

        gpS2->stats(finished->time_);
        gOutfile2 << " ";

        delete finished;
    }
    gLastEventTime = gSimulationTime; // Update time of last event
    delete event;
}

//---------------------------------------------------------------------------
// Arrival Event Handler
//---------------------------------------------------------------------------

class ArrivalHandler : public Handler {
public:
    ~ArrivalHandler ();
    void handle (Event*);
} gArrivalHandler;

// Destructor
ArrivalHandler::~ArrivalHandler() {
}

// Event handler implementation for ArrivalHandler
void ArrivalHandler::handle(Event* event)
{
    if (dFlag > 1) {
        gTee << "\nDEBUG: ArrivalHandler: ";
    }

    // Packet, Px/Py arrives and enters the router queue
    gpRouterQueue->Enqueue((Packet*) event);

    // C-event (conditional event) (C1)
    // Check to see whether router is busy
    if (gpRouter->State() == IDLE) {
        // Condition: packet in router queue and router is idle
        // Change in system state: router takes packet from its queue
        // and starts work.

        if (dFlag > 1) {
            gTee << "{R starts work} ";
        }

        // Schedule the next departure (B3 event).
        double serviceTime;
        !nFlag
            ? serviceTime = gpRandomStream->NextNormal(
                                                 gRouterServiceTime, gSigma)
            : serviceTime = gRouterServiceTime;

        gScheduler.Schedule(&gDepartureHandler,
                               new Event(B3, gSimulationTime + serviceTime));
        gpRouter->State(BUSY); // Update router state

    } else {
        if (dFlag > 1) {
            gTee << "{STATE: R BUSY'} ";
        }
    }

    // Schedule the next arrival of B1/B2 event.
    double interval;
    if (event->type_ == B1) {
        !nFlag
            ? interval = gpRandomStream->Exponential(gPxInterArrivalTime)
            : interval = gPxInterArrivalTime;
    } else if (event->type_ == B2) {
        !nFlag
            ? interval = gpRandomStream->Exponential(gPyInterArrivalTime)
            : interval = gPyInterArrivalTime;
    }

    gScheduler.Schedule(&gArrivalHandler, new Event(event->type_,
                                                gSimulationTime + interval));

    gLastEventTime = gSimulationTime; // Update time of last event
}

//---------------------------------------------------------------------------
// Report performance metrics for the simulation
//---------------------------------------------------------------------------

void Stats::Report () {
    // Compute R mean service time
    if (gpRouter->TotalPacket() > 0) {
        mRMeanServiceTime =
                     gpRouter->TotalServiceTime()/gpRouter->TotalPacket();
    }

    // Compute S1 mean service time
    if (gpS1->TotalPacket() > 0) {
        mS1MeanServiceTime = gpS1->TotalServiceTime()/gpS1->TotalPacket();
    }

    // Compute S2 mean service time
    if (gpS2->TotalPacket() > 0) {
        mS2MeanServiceTime = gpS2->TotalServiceTime()/gpS2->TotalPacket();
    }

    if (dFlag) {
        gTee << "\n"
             << "========================================================="
             << "\n\n";
    }

    gTee << "Performance metrics for the simulation:\n"
         << "========================================================="
         << "\n\n"
         << "Total simulated time = " << gSimulationTime << " sec\n"
         << "Mean interarrival time for Px = "
         << mMeanPxInterArrivalTime << " sec\n"
         << "Mean interarrival time for Py = "
         << mMeanPyInterArrivalTime << " sec\n";

    if (dFlag > 1) {
        gTee << "R (Router):\n"
             << "Total Px served by R = " << mTotalPxOnR
             << ", "
             << "Total Py served by R = " << mTotalPyOnR
             << "\n";
    }

    gTee << "Mean service time for R = "<<mRMeanServiceTime <<" sec\n";

    if (dFlag > 1) {
         gTee << "S1 (Server 1):\n"
              << "\n";
    }

    gTee << "Total Px served by S1 = " <<gpS1->TotalPacket()<<"\n"
         << "Mean service time for S1 = " <<mS1MeanServiceTime<<" sec\n";

    if (dFlag > 1) {
         gTee << "S2 (Server 2):\n"
              << "\n";
    }
    gTee << "Total Py served by S2 = " <<gpS2->TotalPacket()<<"\n"
         << "Mean service time for S2 = " <<mS2MeanServiceTime<<" sec\n\n";
}

//---------------------------------------------------------------------------
// Main program
//---------------------------------------------------------------------------

int main (int argc, char* argv[]) {
    int option = 0;
    string optargstr; // argument option in string
    double endtime = 0; // simulation ending time
    size_t endpx = 0; // simulation ending packet count
    long endpxx = 0;

    // Read the parameters
    while ((option = getopt(argc, argv, "ndht:x:")) != -1) {
        switch (option) {
        case 'h':
            cout << DIAGRAM;
            cout << "\nUsage: " << argv[0] << " [options]\n"
                 << "Options:\n"
                 << "\t-n : do not use random number stream generation\n"
                 << "\t-t : simulation ending time (default 200 s)\n"
                 << "\t-x : simulation ending packet count\n"
                 << "\t-d : increase debugging verbosity (-dd even more)\n"
                 << "\t-h : show this help and exit\n\n";
              return(EXIT_SUCCESS);
        case 't':
            optargstr = optarg;
            if (!optargstr.empty()) {
                istringstream iss(optargstr);
                if( !(iss >> endtime) || endtime <= 0) {
                    cout << argv[0] << ": invalid argument -- '"
                         << optargstr <<"'\n";
                    goto help;
                }
            }
            tFlag++;
            break;
        case 'x':
            optargstr = optarg;
            if (!optargstr.empty()) {
                istringstream iss(optargstr);
                if( !(iss >> endpxx) || endpxx <= 0) {
                    cout << argv[0] << ": invalid argument -- '"
                         << optargstr <<"'\n";
                    goto help;
                }
                endpx = static_cast<size_t>(endpxx);
            }
            xFlag++;
            break;
        case 'd': dFlag++; break;
        case 'n': nFlag++; break;
help:
        default :
                  cout << "Try `" << argv[0]
                       << " -h' for more information.\n";
                  return(EXIT_FAILURE);
        }
    }

    //XXX
    if (tFlag && xFlag) {
        goto help;
    } else if (!tFlag && !xFlag) {
        // Default values
        endtime = 200;
        endpx = 100;
    }

    if (dFlag) {
        cout << "endtime: " << endtime <<"\n";
        cout << "endpx: " << endpx <<"\n";
    }

    // Initialization
    gOutfile1.open("output1.txt");
    gOutfile2.open("output2.txt");
    gSimulationTime = 0;
    gLastEventTime = 0;
    gPxInterArrivalTime = 5;
    gPyInterArrivalTime = 10;
    gRouterServiceTime = 1;
    gS1ServiceTime = 4;
    gS2ServiceTime = 7;
    gSigma = 0.6;
    gSigma1 = 0.6;
    gSigma2 = 0.6;
    gpRouter = new Service;
    gpS1 = new Service;
    gpS2 = new Service;
    gpRouterQueue = new PacketQueue;
    gpS1Queue = new PacketQueue;
    gpS2Queue = new PacketQueue;
    gpStats = new Stats;

    if (!nFlag) {
        // Initialize the psuedo random number generator
        //long seed = 2;
        //srandom(seed); // set the seed value
        srandom(time(NULL));
        gpRandomStream = new RandomStream;
    }
    // Prints the network diagram
    gTee << DIAGRAM;

    // Write header to output file 2
    gOutfile2 << "router,no,type,arrival,begin,service time,"
              << "end,wait in q,spend,idle \\ \n"
              << "  server type,no,type,arrival,begin,service time,"
              << "end,wait in q,spend,idle\n";

    if (dFlag) {
        gTee << "\n" << gSimulationTime << " (initialize simulation) ";
    }

    // Put initial events in FEL
    double interval;
    !nFlag
        ? interval = gpRandomStream->Exponential(gPxInterArrivalTime)
        : interval = gPxInterArrivalTime;

    gScheduler.Schedule(&gArrivalHandler,
                                  new Event(B1, gSimulationTime + interval));
    !nFlag
        ? interval = gpRandomStream->Exponential(gPyInterArrivalTime)
        : interval = gPyInterArrivalTime;

    gScheduler.Schedule(&gArrivalHandler,
                                  new Event(B2, gSimulationTime + interval));

    while (1) {
        // Remove the imminent B-event from FEL
        Event* p = gScheduler.Deque();

        // Advance simulation clock to its event time
        gSimulationTime = p->time_;
        if (dFlag) {
            gTee << "\n" << gSimulationTime
                 << " (Event B" << p->type_ << ") ";
        }

        // Execute all B-type events that were removed from the FEL
        gScheduler.Dispatch(p);

        if (tFlag) {
            if (gSimulationTime >= endtime) {
                break;
            }
        } else if (xFlag) {
            if (gpS1->TotalPacket() == endpx) {
                break;
            }
        } else {
            if (gSimulationTime >= endtime || gpS1->TotalPacket() == endpx) {
                break;
            }
        }
    }

    gTee << "\n";
    gOutfile2 << "\n";
    gpStats->Report();

    // Deallocation routines
    if (!nFlag) {
        delete gpRandomStream;
    }
    delete gpRouter;
    delete gpS1;
    delete gpS2;
    delete gpRouterQueue;
    delete gpS1Queue;
    delete gpS2Queue;
    delete gpStats;

//XXX
//#if defined(_WIN32) || defined(_WIN64)
//    cout << "\nPress enter to exit...\n";
//    std::cin.get();
//#endif
    return(EXIT_SUCCESS);
}
