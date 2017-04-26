	RtString  sAttrValue;
	err = rstate->GetAttribute("identifier:name", &sAttrValue, sizeof(RtString),
                                           &attrType, &attrNumValues);
	if(err == 0 && attrType == RixRenderState::k_StringV) {
		m_msg->Info("String: %s", sAttrValue);
		}
	else
		sAttrValue = NULL;