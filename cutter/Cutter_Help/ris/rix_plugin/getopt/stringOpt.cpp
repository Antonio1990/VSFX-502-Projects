	RtString  sOptValue;
	err = rstate->GetOption("searchpath:shader", &sOptValue, sizeof(RtString),
                                           &optType, &optNumValues);
	if(err == 0 && optType == RixRenderState::k_StringV) {
		m_msg->Info("searchpath:shader: %s", sOptValue);
		}
	else
		sOptValue = NULL;