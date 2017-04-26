    // A regular (non-user) "int" attribute must be read as a RtFloat.
 	RtFloat fAttrValue;
	err = rstate->GetAttribute("trace:maxdiffusedepth", &fAttrValue, sizeof(RtFloat),
                                           &attrType, &attrNumValues);
	if(err == 0 && attrType == RixRenderState::k_Float) {
		m_msg->Info("trace:maxdiffusedepth: %f", fAttrValue);
		}
	else
		fAttrValue = 1.0; // default value ?