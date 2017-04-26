	// Prepare to read 'st' texture values from a connected manifold 
	// or from a 'st' primitive variable. 
	RixSCConnectionInfo cinfo;
	RixSCType type;
	RtPoint3 const *Q;
	RtFloat2 const *st, st_default(0,0);
	
	// Only if the manifold is connected can its Q[i].x and Q[i].y
	// texture coordinates be used.
	ctx->GetParamInfo(k_manifold, &type, &cinfo);
	if(cinfo == k_RixSCNetworkValue) 
		ctx->EvalParam(k_manifoldQ, -1, &Q);
	else
		ctx->GetPrimVar("st", st_default, &st);
	bool	isConnected = (cinfo == k_RixSCNetworkValue) ? true : false;
	int		row, col;
	float	s, t;