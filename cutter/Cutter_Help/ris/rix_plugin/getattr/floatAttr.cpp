 	RtFloat fAttrValue;
	err = rstate->GetAttribute("Ri:Sides", &fAttrValue, sizeof(RtFloat),
                                           &attrType, &attrNumValues);
	if(err == 0 && attrType == RixRenderState::k_Float) {
		m_msg->Info("Sides: %f", fAttrValue);
		}
	else
		fAttrValue = 1.0; // default value ?