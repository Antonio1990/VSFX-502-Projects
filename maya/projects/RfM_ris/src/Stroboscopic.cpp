// Principle code by Patrik Hadron, see
// https://community.renderman.pixar.com/article/1746/creating-a-multiflash-lens.html?l=h
// Adapted for use with an .args file so that the number of flash samples
// can be controlled from Maya's Render Settings "Features", "Projection Modifier" sub-menu.
// Modifications by Malcolm Kesson.
// Feb 2017


#include <RixInterfaces.h>
#include <RixProjection.h>
#include <algorithm>
#include <cmath>

class Stroboscopic : public RixProjection {
	public:
	Stroboscopic();
	virtual ~Stroboscopic() { }
	
	virtual int Init(RixContext& ctx, char const* pluginPath);	
	virtual void Finalize(RixContext& ctx) { }
	virtual RixSCParamInfo const* GetParamTable();
	virtual void RenderBegin(RixContext& ctx, RixProjectionEnvironment& env, RixParameterList const* params);
	virtual void RenderEnd(RixContext& ctx);
	virtual void Synchronize(RixContext& ctx, RixSCSyncMsg syncMsg, RixParameterList const* params);
	virtual void Project(RixProjectionContext& pCtx);
	private:
		int m_numFlashes;
		RixMessages* m_msg;
	};
Stroboscopic::Stroboscopic():
	m_numFlashes(8),
	m_msg(NULL)
	{ }
	
int Stroboscopic::Init(RixContext &ctx, char const *pluginPath ) {
	m_msg = static_cast<RixMessages*>(ctx.GetRixInterface(k_RixMessages));
	return 0;
	}
	
RixSCParamInfo const *Stroboscopic::GetParamTable() {
    static RixSCParamInfo s_ptable[] = {
        RixSCParamInfo( "numFlashes", k_RixSCInteger ),
        RixSCParamInfo() // end of table
		};
	return &s_ptable[ 0 ];
	}
enum paramIndex {
	k_numFlashes
    };
	
void Stroboscopic::RenderBegin(	RixContext& ctx, 
									RixProjectionEnvironment& env, 
									RixParameterList const* params) {
	Synchronize( ctx, k_RixSCInstanceEdit, params );
	}
	
void Stroboscopic::RenderEnd(RixContext& ctx) {
	}

void Stroboscopic::Synchronize(	RixContext &ctx, 
									RixSCSyncMsg syncMsg, 
									RixParameterList const* params) {
	if(syncMsg != k_RixSCInstanceEdit)
		return;
	/*
	RtInt paramId;
	m_numFlashes = 4; // default
	if(params->GetParamId("numFlashes", &paramId ) == 0)
		params->EvalParam(paramId, 0, &m_numFlashes );
	*/	
	params->EvalParam(k_numFlashes, 0, &m_numFlashes);
	}
	
void Stroboscopic::Project(RixProjectionContext &pCtx) {
	//m_msg->Info("m_numFlashes %d\n", m_numFlashes);
	for (int i = 0; i < pCtx.numRays; i++) {
		pCtx.tint[i] = RtColorRGB(3.0f * pCtx.shutter[i] * pCtx.shutter[i]);
		pCtx.shutter[i] = std::floor(pCtx.shutter[i] * m_numFlashes) / (m_numFlashes - 1);
		pCtx.shutter[i] = std::max(0.0f, std::min(1.0f - 1e-7f, pCtx.shutter[i]));
		}
	}
// RIX_PROJECTIONCREATE is a macro for the CreateRixProjection function
RIX_PROJECTIONCREATE {
	return new Stroboscopic;
	}
// RIX_PROJECTIONDESTROY is a macro for the DestroyRixProjection function
RIX_PROJECTIONDESTROY {
	delete reinterpret_cast<Stroboscopic*>(projection);
	}
