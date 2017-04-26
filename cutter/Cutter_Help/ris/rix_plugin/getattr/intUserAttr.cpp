	// A "user" "int" attribute is read as a RtInt.
	RtInt iAttrValue;
	err = rstate->GetAttribute("user:receivesShadows", &iAttrValue, sizeof(RtInt),
                                           &attrType, &attrNumValues);
	if(err == 0 && attrType == RixRenderState::k_Integer) {
		m_msg->Info("user:receivesShadows: %d", iAttrValue);
		}
	else
		iAttrValue = 1.0; // default value ?