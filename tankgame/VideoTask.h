#ifndef VIDEO_TASK_H
#define VIDEO_TASK_H


#include "BaseTask.h"


class VideoTask : public BaseTask  
{
public:
	VideoTask();
	virtual ~VideoTask();

	static int scrWidth, scrHeight, scrBPP;

	bool Start();
	void Update();
	void Stop();

protected:
	int flags;
};

#endif // VIDEO_H
