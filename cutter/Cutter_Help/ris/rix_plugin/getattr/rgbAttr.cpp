 	RtColorRGB  cAttrValue;
	err = rstate->GetAttribute("Ri:Color", &cAttrValue, sizeof(RtColorRGB),
                                           &attrType, &attrNumValues);
	if(err == 0 && attrType == RixRenderState::k_Color) {
		std::stringstream ss; // uncomment #include <sstream>
		ss << cAttrValue;
		m_msg->Info("Color: %s", ss.str().c_str());
		}
	else
		cAttrValue = RtColorRGB(1,1,1); // default value ?