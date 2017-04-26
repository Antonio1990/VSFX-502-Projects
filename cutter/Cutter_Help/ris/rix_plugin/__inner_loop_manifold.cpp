		// For example, assign values based on the value of 's'...
		if(isConnected && Q[i].x > 0.5) {
			resultRGB[i].r = rgb_input[i].r;
			resultRGB[i].g = rgb_input[i].g;
			resultRGB[i].b = rgb_input[i].b;
			}
		else if(st[i].x > 0.5) {
			resultRGB[i].r = rgb_input[i].r;
			resultRGB[i].g = rgb_input[i].g;
			resultRGB[i].b = rgb_input[i].b;
			}
		else {
			resultRGB[i].r = 1;
			resultRGB[i].g = 1;
			resultRGB[i].b = 1;
			}
		}