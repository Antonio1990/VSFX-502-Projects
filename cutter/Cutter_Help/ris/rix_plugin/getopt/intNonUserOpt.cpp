    // A regular (non-user) "int" option must be read as a RtFloat.
 	RtFloat fOptValue;
	err = rstate->GetOption("Frame", &fOptValue, sizeof(RtFloat),
                                           &optType, &optNumValues);
	if(err == 0 && optType == RixRenderState::k_Float) {
		m_msg->Info("Frame: %f", fOptValue);
		}
	else
		fOptValue = 1.0; // default value ?