 	RtColorRGB  cOptValue;
	err = rstate->GetOption("Ri:Color", &cOptValue, sizeof(RtColorRGB),
                                           &optType, &optNumValues);
	if(err == 0 && optType == RixRenderState::k_Color) {
		std::stringstream ss;
		ss << cOptValue;   // uncomment #include <sstream>
		m_msg->Info("Color: %s", ss.str().c_str());
		}
	else
		cOptValue = RtColorRGB(1,1,1); // default value ?