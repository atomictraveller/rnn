#pragma once

char chartonet(char cn) {
	//if (cn == 10) return 0;	//	line feed
	if (cn == 13) return 1;	//	carriage return
	if (cn < 32 || cn > 125) return 0;  //  OOR
	return (cn - 30); //  output 0 to 95
}

char nettochar(char nc) {
	if (nc == 0) return 10;
	if (nc == 1) return 13;
	if (nc < 96) return (nc + 30);
	return 0;
}

void newmodel() {
	//	'xavier' (glorot) initialisation ideal range is +/- sqrt(6/(inputs+outputs))
	//	eg.	sqrt(6/(96 + 256)) = 0.130558
	//	sqrt(6/(256 + 256)) = 0.10825
	for (unsigned int i = 0; i < idim; i++) for (unsigned int j = 0; j < hidd; j++) {
		rnd = 196314165 * rnd + 907633515;  nni[i][j] = (float)(1 + (rnd >> 16)) * 3.967e-6f - 0.13f;
	}
	for (unsigned int i = 0; i < hidd; i++) for (unsigned int j = 0; j < hidd; j++) {
		rnd = 196314165 * rnd + 907633515;  nnh0[i][j] = (float)(1 + (rnd >> 16)) * 3.2959e-6f - 0.108f;
		rnd = 196314165 * rnd + 907633515;  nnh0h[i][j] = (float)(1 + (rnd >> 16)) * 3.2959e-6f - 0.108f;
		rnd = 196314165 * rnd + 907633515;  nnh[i][j] = (float)(1 + (rnd >> 16)) * 3.2959e-6f - 0.108f;
	}
	for (unsigned int i = 0; i < hidd; i++) for (unsigned int j = 0; j < idim; j++) {
		rnd = 196314165 * rnd + 907633515;  nno[i][j] = (float)(1 + (rnd >> 16)) * 3.967e-6f - 0.13f;
	}

	memset(hbias, 0, sizeof h0bias);	memset(hbias, 0, sizeof hbias);	memset(obias, 0, sizeof obias);
	for (unsigned int i = 0; i < winp; i++) for (unsigned int j = 0; j < hidd; j++) h0[i][j] = h[i][j] = 0;
}

void openmodel() {
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr)) {
		IFileOpenDialog* pFileOpen;
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
		if (SUCCEEDED(hr)) {
			COMDLG_FILTERSPEC ComDlgFS[1] = { {L"data files", L"*.dat"} };
			hr = pFileOpen->SetFileTypes(1, ComDlgFS);
			//pFileOpen->SetTitle(L"A Single Selection Dialog");
			hr = pFileOpen->Show(NULL);
			if (SUCCEEDED(hr)) {
				IShellItem* pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr)) {
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					if (SUCCEEDED(hr)) {
						std::ifstream infile(pszFilePath, std::ios::binary);
						infile.read(reinterpret_cast<char*>(&nni), sizeof(nni));
						infile.read(reinterpret_cast<char*>(&nnh0), sizeof(nnh0));
						infile.read(reinterpret_cast<char*>(&nnh0h), sizeof(nnh0h));
						infile.read(reinterpret_cast<char*>(&nnh), sizeof(nnh));
						infile.read(reinterpret_cast<char*>(&nno), sizeof(nno));
						infile.read(reinterpret_cast<char*>(&h0bias), sizeof(h0bias));
						infile.read(reinterpret_cast<char*>(&hbias), sizeof(hbias));
						infile.read(reinterpret_cast<char*>(&obias), sizeof(obias));
						infile.close();
						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}   pFileOpen->Release();
			}
		}   CoUninitialize();
	}
}

void savemodel() {
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr)) {
		IFileSaveDialog* pFileSave;
		hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
			IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));
		if (SUCCEEDED(hr)) {
			COMDLG_FILTERSPEC ComDlgFS[1] = { {L"data files", L"*.dat"} };
			hr = pFileSave->SetFileTypes(1, ComDlgFS);
			hr = pFileSave->Show(NULL);
			if (SUCCEEDED(hr)) {
				IShellItem* pItem;
				hr = pFileSave->GetResult(&pItem);
				if (SUCCEEDED(hr)) {
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					if (SUCCEEDED(hr)) {
						// try using fwrite (C) sometime 
						if (!wcsrchr(pszFilePath, L'.')) lstrcat(pszFilePath, L".dat");
						std::ofstream out(pszFilePath, std::ios::binary);
						out.write(reinterpret_cast<char*>(&nni), sizeof(nni));
						out.write(reinterpret_cast<char*>(&nnh0), sizeof(nnh0));
						out.write(reinterpret_cast<char*>(&nnh0h), sizeof(nnh0h));
						out.write(reinterpret_cast<char*>(&nnh), sizeof(nnh));
						out.write(reinterpret_cast<char*>(&nno), sizeof(nno));
						out.write(reinterpret_cast<char*>(&h0bias), sizeof(h0bias));
						out.write(reinterpret_cast<char*>(&hbias), sizeof(hbias));
						out.write(reinterpret_cast<char*>(&obias), sizeof(obias));
						out.close();
						CoTaskMemFree(pszFilePath);
					}   pItem->Release();
				}
			}   pFileSave->Release();
		}   CoUninitialize();
	}
}




void sampleoutput(HWND hWnd) {
	memset(outext, 0, sizeof outext);

	for (tests = 0; tests < min(512, strlen(intext)); tests++) {
		char twin = tests & winm;
		char tprev = (twin + winm) & winm;	//	twin previous step

		b = chartonet(intext[tests]);
		for (unsigned int i = 0; i < hidd; i++) {   //  forward pass
			float sum = h0bias[i];
			sum += nni[b][i];  //  "hot one"
			for (unsigned int j = 0; j < hidd; j++) sum += nnh0[i][j] * h0[tprev][j];
			h0[twin][i] = tanhf(sum);
		}
		for (unsigned int i = 0; i < hidd; i++) {
			float sum = hbias[i];
			for (unsigned int j = 0; j < hidd; j++) sum += nnh0h[i][j] * h0[twin][j];
			for (unsigned int j = 0; j < hidd; j++) sum += nnh[i][j] * h[tprev][j];
			h[twin][i] = tanhf(sum);
		}
		for (int i = 0; i < idim; i++) {
			float sum = obias[i];
			for (unsigned int j = 0; j < hidd; j++) sum += nno[j][i] * h[twin][j];
			netout[twin][i] = sum;
		}
	}
	{
		char twin = max(0, tests - 1);	twin &= winm;	//	assign prediction from last input step to b
		float r = 0.f;
		for (unsigned int i = 0; i < idim; i++) {
			if (netout[twin][i] > r) { r = netout[twin][i]; b = i; }
		}
	}
	c = nettochar(b);
	if (c > 31 && c < 126) outext[0][0] = c;

	tests = 1;
	while (tests < 1024) {
		char twin = tests & winm;
		char tprev = (twin + winm) & winm;	//	twin previous step

		for (unsigned int i = 0; i < hidd; i++) {   //  forward pass
			float sum = h0bias[i];
			sum += nni[b][i];  //  "hot one"
			for (unsigned int j = 0; j < hidd; j++) sum += nnh0[i][j] * h0[tprev][j];
			h0[twin][i] = tanhf(sum);
		}
		for (unsigned int i = 0; i < hidd; i++) {
			float sum = hbias[i];
			for (unsigned int j = 0; j < hidd; j++) sum += nnh0h[i][j] * h0[twin][j];
			for (unsigned int j = 0; j < hidd; j++) sum += nnh[i][j] * h[tprev][j];
			h[twin][i] = tanhf(sum);
		}
		float r = 0.f;	//	check moved up for generation
		for (int i = 0; i < idim; i++) {
			float sum = obias[i];
			for (unsigned int j = 0; j < hidd; j++) sum += nno[j][i] * h[twin][j];
			netout[twin][i] = sum;	if (sum > r) { r = sum; b = i; }	//	assign b for next step
		}

		c = nettochar(b);
		int twrap = tests & 127;
		if (twrap > 118) {
			if ((b == 1 || b == 2) && tests < 896) {
				tests += (128 - twrap);	twrap = 0;
			}
		}
		if (c > 31 && c < 126) outext[(tests >> 7)][twrap] = c;

		//			if (!(tests & 255)) {

		InvalidateRgn(hWnd, 0, 0);	UpdateWindow(hWnd);
		MSG tmsg;
		while (PeekMessage(&tmsg, hWnd, 0, 0, PM_REMOVE)) { TranslateMessage(&tmsg); DispatchMessage(&tmsg); }
		//			}
		//		}

		tests++;
	}
}





void train(HWND hWnd) {
	thismany = 0;	adamt = 1;
	while (thismany < 1 + ((int)do10x * 9)) {
		for (unsigned int i = 0; i < winp; i++) for (unsigned int j = 0; j < hidd; j++) h0[i][j] = h[i][j] = 0;
		dispix++;	if (dispix > 9) dispix = 0;	//	for stepping through display colors
		ebuf = 0.f;
		tests = 0;
		b = 2;
		for (unsigned int i = 0; i < 127; i++) out0[i] = out1[i] = 32;
		std::ifstream ifile("train.txt");
		while (ifile.get(c)) {
			char tdisp = tests & 127;	//	for display
			char twin = tests & winm;	//	truncated rnn windowing for indefinite serial processing
			char tprev = (twin + winm) & winm;	//	twin previous step

			char tc = chartonet(c);		//  "target/next char" (current read) "correct prediction"
			cin[twin] = b;	//memset(netin, 0, sizeof netin);	netin[b] = 1.f;			//  "prev char" (previous read) "current step"

			for (unsigned int i = 0; i < hidd; i++) {   //  forward pass
				float sum = h0bias[i];
				sum += nni[b][i];  //  "hot one"
				for (unsigned int j = 0; j < hidd; j++) sum += nnh0[i][j] * h0[tprev][j];
				h0[twin][i] = tanhf(sum);
			}
			for (unsigned int i = 0; i < hidd; i++) {
				float sum = hbias[i];
				for (unsigned int j = 0; j < hidd; j++) sum += nnh0h[i][j] * h0[twin][j];
				for (unsigned int j = 0; j < hidd; j++) sum += nnh[i][j] * h[tprev][j];
				h[twin][i] = tanhf(sum);
			}
			float m = -1e9f;
			for (int i = 0; i < idim; i++) {
				float sum = obias[i];
				for (unsigned int j = 0; j < hidd; j++) sum += nno[j][i] * h[twin][j];
				netout[twin][i] = sum;    m = fmax(m, netout[twin][i]);
			}
			float ssum = 0.f;
			for (unsigned int i = 0; i < idim; i++) {
				netout[twin][i] = exp(netout[twin][i] - m);	ssum += netout[twin][i];
			}
			softsum[twin] = 0;
			if (ssum > 0.f) {
				softsum[twin] = ssum; ssum = 1.f / ssum;
				for (unsigned int i = 0; i < idim; i++) netout[twin][i] *= ssum;
			}
			ebuf -= log(fmax(1e-18f, netout[twin][tc]));	//	-logf(netout[twin][tc] + 1e-12f);
			edisp = ebuf / (float)max(1, tests);

			float r = 0.f;
			char ri = 0;
			for (unsigned int i = 0; i < idim; i++) {
				if (netout[twin][i] > r) { r = netout[twin][i]; ri = i; }
			}
			ri = nettochar(ri);

			netout[twin][tc] -= 1.f;


			if (twin == winm) {	//	eg. "63 of 63" do BPTT back propogation through time
				memset(dnni, 0, sizeof dnni);	//	gradients
				memset(dnnh0, 0, sizeof dnnh0);	memset(dnnh0h, 0, sizeof dnnh0h);	memset(dnnh, 0, sizeof dnnh);
				memset(dnno, 0, sizeof dnno);
				memset(dh0bias, 0, sizeof dh0bias);	memset(dhbias, 0, sizeof dhbias);	memset(dobias, 0, sizeof dobias);
				memset(dh0_next, 0, sizeof dh0_next);	memset(dh_next, 0, sizeof dh_next);

				for (int iter = winm; iter > -1; iter--) {		//	BPTT 'back propogation through time'
					int iprev = (bool)iter ? iter - 1 : wind;	//	(iter + winm) & winm;
					for (unsigned int i = 0; i < idim; i++) {
						dobias[i] += netout[iter][i];
						for (unsigned int j = 0; j < hidd; j++) dnno[j][i] += netout[iter][i] * h[iter][j];
					}
					for (int i = 0; i < hidd; i++) {
						float sum = dh_next[i];
						for (int j = 0; j < idim; j++) sum += nno[i][j] * netout[iter][j];
						dh[i] = sum;
					}
					for (int i = 0; i < hidd; i++) {
						dh_raw[i] = (1.f - (h[iter][i] * h[iter][i])) * dh[i];
						dhbias[i] += dh_raw[i];
						for (unsigned int j = 0; j < hidd; j++) dnnh0h[i][j] += dh_raw[i] * h0[iter][j];	//	layer transference, iter
						for (unsigned int j = 0; j < hidd; j++) dnnh[i][j] += dh_raw[i] * h[iprev][j];
					}
					for (int i = 0; i < hidd; i++) {
						float sum = dh0_next[i];
						for (int j = 0; j < hidd; j++) sum += nnh0h[j][i] * dh_raw[j];	//	ji right way :)
						dh0[i] = sum;
					}
					for (int i = 0; i < hidd; i++) {
						dh0_raw[i] = (1.f - (h0[iter][i] * h0[iter][i])) * dh0[i];
						dh0bias[i] += dh0_raw[i];
						dnni[cin[iter]][i] += dh0_raw[i];
						for (unsigned int j = 0; j < hidd; j++) dnnh0[i][j] += dh0_raw[i] * h0[iprev][j];
					}
					for (int i = 0; i < hidd; i++) {
						float sum0 = 0.f;    float sum = 0.f;
						for (int j = 0; j < hidd; j++) {
							sum0 += nnh0[j][i] * dh0_raw[j];
							sum += nnh[j][i] * dh_raw[j];
						}
						dh0_next[i] = sum0;
						dh_next[i] = sum;
					}
				}
				memcpy(h0[wind], h0[winm], sizeof h0[wind]);	//	after tBPTT copy next round's h[-1] to h[wind]
				memcpy(h[wind], h[winm], sizeof h[wind]);		//	to solve buffer for windowed forwards and backwards processing

				if (GRADNORM) {
					float norm = 0.f;		//	gradient normalisation
					float clip = 1.f;

					for (int i = 0; i < idim; i++) {
						for (int j = 0; j < hidd; j++) norm += dnni[i][j] * dnni[i][j];
						norm += dobias[i] * dobias[i];
					}
					for (int i = 0; i < hidd; i++) {
						for (int j = 0; j < hidd; j++) {
							norm += dnnh0[i][j] * dnnh0[i][j];
							norm += dnnh0h[i][j] * dnnh0h[i][j];
							norm += dnnh[i][j] * dnnh[i][j];
						}
						for (int j = 0; j < idim; j++) norm += dnno[i][j] * dnno[i][j];
						norm += dh0bias[i] * dh0bias[i];
						norm += dhbias[i] * dhbias[i];
					}
					if (norm > clip) {
						norm = clip / sqrt(norm);
						for (int i = 0; i < idim; i++) {
							for (int j = 0; j < hidd; j++) dnni[i][j] *= norm;
							dobias[i] *= norm;
						}
						for (int i = 0; i < hidd; i++) {
							for (int j = 0; j < hidd; j++) {
								dnnh0[i][j] *= norm;
								dnnh0h[i][j] *= norm;
								dnnh[i][j] *= norm;
							}
							for (int j = 0; j < idim; j++) dnno[i][j] *= norm;
							dh0bias[i] *= norm;
							dhbias[i] *= norm;
						}
					}
				}
				else {	//	else clip gradients
					register float clip = 1.f;
					for (int i = 0; i < idim; i++) {	//	gradient clipping
						for (int j = 0; j < hidd; j++) {
							dnni[i][j] = fmax(-clip, fmin(clip, dnni[i][j]));
							dnno[j][i] = fmax(-clip, fmin(clip, dnno[j][i]));
						}
						dobias[i] = fmax(-clip, fmin(clip, dobias[i]));
					}
					for (int i = 0; i < hidd; i++) {
						dh0bias[i] = fmax(-clip, fmin(clip, dh0bias[i]));
						dhbias[i] = fmax(-clip, fmin(clip, dhbias[i]));
						for (int j = 0; j < hidd; j++) {
							dnnh0[i][j] = fmax(-clip, fmin(clip, dnnh0[i][j]));
							dnnh0h[i][j] = fmax(-clip, fmin(clip, dnnh0h[i][j]));
							dnnh[i][j] = fmax(-clip, fmin(clip, dnnh[i][j]));
						}
					}
				}

				register float learn = learnarr[learnix] * temparr[tempix];
				if (ADAM) {
					float mbiascorr = (float)(1.0 / (1.0 - pow((double)beta1, adamt)));
					float vbiascorr = (float)(1.0 / (1.0 - pow((double)beta2, adamt)));
					adamt++;
					learn *= 0.125f;

					for (unsigned int i = 0; i < hidd; i++) {   // update weights using ADAM optimisation	
						register float vcorr;
						for (unsigned int j = 0; j < idim; j++) {
							mnno[i][j] += beta1m * (dnno[i][j] - mnno[i][j]);
							vnno[i][j] += beta2m * (dnno[i][j] * dnno[i][j] - vnno[i][j]);
							vcorr = vnno[i][j] * vbiascorr;
							nno[i][j] -= learn * mnno[i][j] * mbiascorr / (sqrt(vcorr) + epsilon);
							mnni[j][i] += beta1m * (dnni[j][i] - mnni[j][i]);
							vnni[j][i] += beta2m * (dnni[j][i] * dnni[j][i] - vnni[j][i]);
							vcorr = vnni[j][i] * vbiascorr;
							nni[j][i] -= learn * mnni[j][i] * mbiascorr / (sqrt(vcorr) + epsilon);
						}
						for (unsigned int j = 0; j < hidd; j++) {
							mnnh0[i][j] += beta1m * (dnnh0[i][j] - mnnh0[i][j]);
							vnnh0[i][j] += beta2m * (dnnh0[i][j] * dnnh0[i][j] - vnnh0[i][j]);
							vcorr = vnnh0[i][j] * vbiascorr;
							nnh0[i][j] -= learn * mnnh0[i][j] * mbiascorr / (sqrt(vcorr) + epsilon);
							mnnh0h[i][j] += beta1m * (dnnh0h[i][j] - mnnh0h[i][j]);
							vnnh0h[i][j] += beta2m * (dnnh0h[i][j] * dnnh0h[i][j] - vnnh0h[i][j]);
							vcorr = vnnh0h[i][j] * vbiascorr;
							nnh0h[i][j] -= learn * mnnh0h[i][j] * mbiascorr / (sqrt(vcorr) + epsilon);
							mnnh[i][j] += beta1m * (dnnh[i][j] - mnnh[i][j]);
							vnnh[i][j] += beta2m * (dnnh[i][j] * dnnh[i][j] - vnnh[i][j]);
							vcorr = vnnh[i][j] * vbiascorr;
							nnh[i][j] -= learn * mnnh[i][j] * mbiascorr / (sqrt(vcorr) + epsilon);
						}
						mh0bias[i] += beta1m * (dh0bias[i] - mh0bias[i]);
						vh0bias[i] += beta2m * (dh0bias[i] * dh0bias[i] - vh0bias[i]);
						vcorr = vh0bias[i] * vbiascorr;
						h0bias[i] -= learn * mh0bias[i] * mbiascorr / (sqrt(vcorr) + epsilon);
						mhbias[i] += beta1m * (dhbias[i] - mhbias[i]);
						vhbias[i] += beta2m * (dhbias[i] * dhbias[i] - vhbias[i]);
						vcorr = vhbias[i] * vbiascorr;
						hbias[i] -= learn * mhbias[i] * mbiascorr / (sqrt(vcorr) + epsilon);
					}
					for (unsigned int i = 0; i < idim; i++) {
						mobias[i] += beta1m * (dobias[i] - mobias[i]);
						vobias[i] += beta2m * (dobias[i] * dobias[i] - vobias[i]);
						float vcorr = vobias[i] * vbiascorr;
						obias[i] -= learn * mobias[i] * mbiascorr / (sqrt(vcorr) + epsilon);
					}
				}
				else {
					for (int i = 0; i < idim; i++) {	//	apply learning no ADAM
						for (int j = 0; j < hidd; j++) {
							nni[i][j] -= dnni[i][j] * learn;
							nno[j][i] -= dnno[j][i] * learn;
						}
						obias[i] -= dobias[i] * learn;
					}
					for (int i = 0; i < hidd; i++) {
						h0bias[i] -= dh0bias[i] * learn;
						hbias[i] -= dhbias[i] * learn;
						for (int j = 0; j < hidd; j++) {
							nnh0[i][j] -= dnnh0[i][j] * learn;
							nnh0h[i][j] -= dnnh0h[i][j] * learn;
							nnh[i][j] -= dnnh[i][j] * learn;
						}
					}
				}

				if (regix < 30) {
					float l2nni = 0, l2nnh0 = 0, l2nnh0h = 0, l2nnh = 0, l2nno = 0, l2h0bias = 0, l2hbias = 0, l2obias = 0;	//	L2 regularisation
					for (int i = 0; i < idim; i++) {
						for (int j = 0; j < hidd; j++) {
							l2nni += nni[i][j] * nni[i][j];
							l2nno += nno[j][i] * nno[j][i];
						}
						l2obias += obias[i] * dobias[i];
					}
					for (int i = 0; i < hidd; i++) {
						l2h0bias += h0bias[i] * h0bias[i];
						l2hbias += hbias[i] * hbias[i];
						for (int j = 0; j < hidd; j++) {
							l2nnh0 += nnh0[i][j] * nnh0[i][j];
							l2nnh0h += nnh0h[i][j] * nnh0h[i][j];
							l2nnh += nnh[i][j] * nnh[i][j];
						}
					}
					l2nni *= regarr[regix];		l2nno *= regarr[regix];		l2obias *= regarr[regix];
					l2h0bias *= regarr[regix];	l2hbias *= regarr[regix];	l2nnh *= regarr[regix];
					l2nnh0 *= regarr[regix];	l2nnh0h *= regarr[regix];

					for (int i = 0; i < idim; i++) {
						for (int j = 0; j < hidd; j++) {
							nni[i][j] -= l2nni * nni[i][j];
							nno[j][i] -= l2nno * nno[j][i];
						}
						obias[i] -= l2obias * obias[i];
					}
					for (int i = 0; i < hidd; i++) {
						h0bias[i] -= l2h0bias * h0bias[i];
						hbias[i] -= l2hbias * hbias[i];
						for (int j = 0; j < hidd; j++) {
							nnh0[i][j] -= l2nnh0 * nnh0[i][j];
							nnh0h[i][j] -= l2nnh0h * nnh0h[i][j];
							nnh[i][j] -= l2nnh * nnh[i][j];
						}
					}
				}
			}
			tests++;
			b = tc;


			out0[tdisp] = c;	//	to display data
			out1[tdisp] = ri;	//	to display predictions

			if (!(tests & 255)) {
				if (stop) {
					ifile.close();  stop = 0;   thismany = 10;
				}
				InvalidateRgn(hWnd, 0, 0);	UpdateWindow(hWnd);
				MSG tmsg;
				while (PeekMessage(&tmsg, hWnd, 0, 0, PM_REMOVE)) { TranslateMessage(&tmsg); DispatchMessage(&tmsg); }
			}
		}
		int lastc = ((tests & winm) + winp) & winm;
		memcpy(h[0], h[lastc], sizeof h[0]);
		ifile.close();
		thismany++;
	}
}


