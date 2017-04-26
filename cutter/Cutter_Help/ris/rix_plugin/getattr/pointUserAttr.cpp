		RtPoint3 pAttrValue;
		err = rstate->GetAttribute("user:position", &pAttrValue, sizeof(RtPoint3),
									&attrType, &attrNumValues);
		if(err == 0 && attrType == RixRenderState::k_Point) {
			// Be careful - this might echo very many values 
			m_msg->Info("pAttrValue: %1.3f %1.3f %1.3f", pAttrValue[0],pAttrValue[1],pAttrValue[2]);
			}
		else
			pAttrValue = RtPoint3(0,0,0); // default value ?