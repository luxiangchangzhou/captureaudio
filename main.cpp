
#include <Mmdeviceapi.h>
#include <Audioclient.h>
#include <Audiopolicy.h>
#include <Devicetopology.h>
#include <Endpointvolume.h>

#include <iostream>
#include <stdio.h>

using namespace std;

//-----------------------------------------------------------
// Record an audio stream from the default audio capture
// device. The RecordAudioStream function allocates a shared
// buffer big enough to hold one second of PCM audio data.
// The function uses this buffer to stream data from the
// capture device. The main loop runs every 1/2 second.
//-----------------------------------------------------------

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);





int main()
{
	
	FILE * fp = fopen("lx.pcm", "wb+");
	if (fp == 0)
	{
		cout << "open failed................................" << endl;
	}
	
	int write_count = 0;















	HRESULT hr;
	REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
	REFERENCE_TIME hnsActualDuration;
	UINT32 bufferFrameCount;
	UINT32 numFramesAvailable;
	IMMDeviceEnumerator *pEnumerator = NULL;
	IMMDevice *pDevice = NULL;
	IAudioClient *pAudioClient = NULL;
	IAudioCaptureClient *pCaptureClient = NULL;
	WAVEFORMATEX *pwfx = NULL;
	WAVEFORMATEXTENSIBLE * pwfxex = NULL;
	UINT32 packetLength = 0;
	BOOL bDone = FALSE;
	BYTE *pData;
	DWORD flags;


	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	EXIT_ON_ERROR(hr)

	hr = CoCreateInstance(
		CLSID_MMDeviceEnumerator, NULL,
		CLSCTX_ALL, IID_IMMDeviceEnumerator,
		(void**)&pEnumerator);
	EXIT_ON_ERROR(hr)

	hr = pEnumerator->GetDefaultAudioEndpoint(
		eRender, eConsole, &pDevice);
	EXIT_ON_ERROR(hr)

	hr = pDevice->Activate(
			IID_IAudioClient, CLSCTX_ALL,
			NULL, (void**)&pAudioClient);
	EXIT_ON_ERROR(hr)

	hr = pAudioClient->GetMixFormat(&pwfx);
	EXIT_ON_ERROR(hr);	
		
	//luxiang+
	//����Ĵ������кܶ�ģ���ĵط�,���ڻ���ȷ������,������ʽwindowsʼ��ʹ��float,
	//�ĵ���˵���Ը�,�������ø���,�Ͼ�aac�õľ���float
	cout << "pwfx->wFormatTag = " << pwfx->wFormatTag << endl;//#define WAVE_FORMAT_PCM     1   #define  WAVE_FORMAT_EXTENSIBLE                 0xFFFE
	cout << "pwfx->nChannels = " << pwfx->nChannels << endl;
	cout << "pwfx->nSamplesPerSec = " << pwfx->nSamplesPerSec << endl;
	cout << "pwfx->nAvgBytesPerSec = " << pwfx->nAvgBytesPerSec << endl;
	cout << "pwfx->nBlockAlign = " << pwfx->nBlockAlign << endl;
	cout << "pwfx->wBitsPerSample = " << pwfx->wBitsPerSample << endl;//the container size, not necessarily the sample size
	cout << "pwfx->cbSize = " << pwfx->cbSize << endl;

	if (pwfx->wFormatTag == WAVE_FORMAT_PCM)
	{
	}
	else if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
	{
		pwfxex = (WAVEFORMATEXTENSIBLE *)pwfx;
		cout << "waoooo-----------------using WAVE_FORMAT_EXTENSIBLE" << endl;
		cout << "wValidBitsPerSample = " << pwfxex->Samples.wValidBitsPerSample << endl;
		cout << "wSamplesPerBlock = " << pwfxex->Samples.wSamplesPerBlock << endl;
		cout << "dwChannelMask = " << pwfxex->dwChannelMask << endl;

	}
	////////////////////////////////////////////////////////////////////////


	hr = pAudioClient->Initialize(
			AUDCLNT_SHAREMODE_SHARED,
		AUDCLNT_STREAMFLAGS_LOOPBACK,
			hnsRequestedDuration,
			0,
			pwfx,
			NULL);
	EXIT_ON_ERROR(hr)

	// Get the size of the allocated buffer.
	hr = pAudioClient->GetBufferSize(&bufferFrameCount);
	EXIT_ON_ERROR(hr)

	hr = pAudioClient->GetService(IID_IAudioCaptureClient,(void**)&pCaptureClient);
	EXIT_ON_ERROR(hr)

	// Notify the audio sink which format to use.
	//hr = pMySink->SetFormat(pwfx);
	EXIT_ON_ERROR(hr)

	// Calculate the actual duration of the allocated buffer.
	hnsActualDuration = (double)REFTIMES_PER_SEC *
	bufferFrameCount / pwfx->nSamplesPerSec;

	hr = pAudioClient->Start();  // Start recording.
	EXIT_ON_ERROR(hr)


	//���ڵ������������ȥ�������׵�����
	//Ŀǰ��˼·�Ǹ�ȫΪ0������(ͨ�����Է���ֻҪ���ݶ�һ��ûɶ����),������,��pcm���ݻ�����ⲻ��~~~
	float * pf = new float[pwfx->nChannels * pwfx->nSamplesPerSec*10];
	memset(pf, 0, pwfx->nChannels * pwfx->nSamplesPerSec * 10 * sizeof(float));
	int float_data_count_persecond = pwfx->nChannels * pwfx->nSamplesPerSec;
	int float_data_count = float_data_count_persecond * hnsActualDuration / REFTIMES_PER_MILLISEC / 2 / 1000;



	// Each loop fills about half of the shared buffer.
	while (bDone == FALSE)
	{
		// Sleep for half the buffer duration.
		Sleep(hnsActualDuration / REFTIMES_PER_MILLISEC / 2);

		hr = pCaptureClient->GetNextPacketSize(&packetLength);
		EXIT_ON_ERROR(hr)

		if (packetLength == 0)
		{
			cout << "packetLength == 0 ************************************" << endl;
			//���ʱ�����Ҫд������Ƶ���ݽ�ȥ��,Ӧ��д�����Ƶ���ݵ�������  (hnsActualDuration / REFTIMES_PER_MILLISEC / 2) ʱ���ڵ���
			fwrite(pf, sizeof(float), float_data_count, fp);

		}
		

		while (packetLength != 0)
		{
			cout << "packetLength != 0 ---------------------------------------" << endl;
			// Get the available data in the shared buffer.
			hr = pCaptureClient->GetBuffer(
				&pData,
				&numFramesAvailable,
				&flags, NULL, NULL);
			EXIT_ON_ERROR(hr)

			cout << "packetLength = " << packetLength << endl;
			cout << "numFramesAvailable = " << numFramesAvailable << endl;

			if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
			{
				cout << "AUDCLNT_BUFFERFLAGS_SILENT" << endl;
				pData = NULL;  // Tell CopyData to write silence.
			}

			// Copy the available capture data to the audio sink.
			//hr = pMySink->CopyData(pData, numFramesAvailable, &bDone);

			fwrite(pData, numFramesAvailable*pwfx->nBlockAlign, 1, fp);
			write_count++;






			EXIT_ON_ERROR(hr)

			hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
			EXIT_ON_ERROR(hr)

			hr = pCaptureClient->GetNextPacketSize(&packetLength);
			EXIT_ON_ERROR(hr)


			if (write_count > 3000)
			{
				bDone = true;
				fclose(fp);
				break;
			}



		}
	}
	delete[] pf;

	cout << "over" << endl;

	hr = pAudioClient->Stop();  // Stop recording.
	EXIT_ON_ERROR(hr)

Exit:
	CoTaskMemFree(pwfx);
	SAFE_RELEASE(pEnumerator)
	SAFE_RELEASE(pDevice)
	SAFE_RELEASE(pAudioClient)
	SAFE_RELEASE(pCaptureClient)


	return 0;
}


