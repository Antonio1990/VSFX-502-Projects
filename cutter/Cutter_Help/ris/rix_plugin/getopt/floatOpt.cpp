 	RtFloat fOptValue;
	err = rstate->GetOption("hair:minwidth", &fOptValue, sizeof(RtFloat),
                                           &optType, &optNumValues);
	if(err == 0 && optType == RixRenderState::k_Float) {
		m_msg->Info("hair:minwidth: %f", fOptValue);
		}
	else
		fOptValue = 1.0; // default value ?