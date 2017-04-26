/*
Malcolm Kesson
March 9 2017
*/
#include <RifPlugin.h>
#include <RifFilter.h>
#include <ri.h>
#include <string>
#include <rx.h>
#include <sstream>
class NoTranslationBlur : public RifPlugin {
	public:
	     	 NoTranslationBlur();
	virtual ~NoTranslationBlur() { }
	virtual RifFilter &GetFilter();
	
	private:
		RifFilter   m_filters;
		bool 		m_motionBlockIsOpen;
		int 		m_motionNumSamples;
		int 		m_motionSample;
		RtFloat 	m_xyz[3];

		// Callback_____________________________________________________________
		static RtVoid   motionBegin(int n, float times[]);
		static RtVoid   concatTransform(RtMatrix transform);
		static RtVoid   motionEnd();
	};

RifPlugin *RifPluginManufacture(int argc, char **argv) {
    return new NoTranslationBlur();
	}
		
NoTranslationBlur::NoTranslationBlur() {
	m_motionBlockIsOpen = false;
	m_motionNumSamples = 0;
	m_motionSample = 0;
	m_filters.MotionBeginV = motionBegin;
	m_filters.ConcatTransform = concatTransform;
	m_filters.MotionEnd = motionEnd;
	m_filters.Filtering = RifFilter::k_Continue;
	}

RifFilter& NoTranslationBlur::GetFilter() { 
	return m_filters; 
	}
	
RtVoid NoTranslationBlur::motionBegin(int n, float times[]) {
	NoTranslationBlur *self = static_cast<NoTranslationBlur*> (RifGetCurrentPlugin());
    self->m_motionNumSamples = n;
	self->m_motionBlockIsOpen = true;
	self->m_motionSample = 1;
	RiMotionBeginV(n, times);
	}
RtVoid NoTranslationBlur::concatTransform(RtMatrix transform) {
	NoTranslationBlur *self = static_cast<NoTranslationBlur*> (RifGetCurrentPlugin());
	if(self->m_motionBlockIsOpen == false) {
		RiConcatTransform(transform);
		return;
		}
	// Store the x,y,z values
	if(self->m_motionSample == 1) {
		self->m_xyz[0] = transform[3][0];
		self->m_xyz[1] = transform[3][1];
		self->m_xyz[2] = transform[3][2];
		//std::stringstream ss;
		//ss << transform[3][0];
		//RiArchiveRecord(RI_COMMENT, (char*)ss.str().c_str());
		self->m_motionSample += 1;
		}
	else
		{
		// Overwrite the x,y,z values
		transform[3][0] = self->m_xyz[0];
		transform[3][1] = self->m_xyz[1];
		transform[3][2] = self->m_xyz[2];
		}
	RiConcatTransform(transform);
	}
RtVoid NoTranslationBlur::motionEnd() {
	NoTranslationBlur *self = static_cast<NoTranslationBlur*> (RifGetCurrentPlugin());
	self->m_motionBlockIsOpen = false;
	self->m_motionNumSamples = 0;
	RiMotionEnd();
	}



