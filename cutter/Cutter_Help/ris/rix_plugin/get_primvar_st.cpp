	// Access the primitive variables that will be needed for the 
	// calculation of the output values. For example, 'st' texture
	// values.
	RtFloat2 const *st, st_default(0, 0);
	ctx->GetPrimVar("st", st_default, &st);	