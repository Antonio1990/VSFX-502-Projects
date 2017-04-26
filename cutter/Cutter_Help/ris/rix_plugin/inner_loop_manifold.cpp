		s = (isConnected) ? Q[i].x : st[i].x;
		t = (isConnected) ? Q[i].y : st[i].y;
		row = int(t);
		col = int(s);
		s = s - col;
		t = t - row;
		// For example, assign output based on the value of 's'
		if(s > 0.5) {