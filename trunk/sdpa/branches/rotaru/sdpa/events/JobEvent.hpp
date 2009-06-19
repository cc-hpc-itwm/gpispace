#ifndef JOBEVENT_H
#define JOBEVENT_H

struct JobEvent {
	JobEvent(int id = -1 ) { m_nJobID = id; }
	virtual ~JobEvent() {}
    void SetJobID( int id ) { m_nJobID = id; }
    int GetJobID() const { return m_nJobID; }
private:
	int m_nJobID;
};

#endif
