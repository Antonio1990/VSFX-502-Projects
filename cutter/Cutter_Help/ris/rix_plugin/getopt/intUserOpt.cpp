    // A "user" "int" option is read as a RtInt.
 	RtInt iOptValue;
	err = rstate->GetOption("user:input_color_profile", &iOptValue, sizeof(RtInt),
                                           &optType, &optNumValues);
	if(err == 0 && optType == RixRenderState::k_Integer) {
		m_msg->Info("user:profile: %d", iOptValue);
		}
	else
		iOptValue = 1.0; // default value ?