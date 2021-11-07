#ifndef _TRACE_H
#define _TRACE_H

class Trace {
public:	
	Trace() {}
	~Trace() {}

	void end_of_computation();
	void end_of_strand();
};


extern Trace global_trace;

#endif
