#ifndef PWM_SHARED_H
#define PWM_SHARED_H

#ifdef WIN32
	#ifdef PWM_CORE_EXPORT
		#define PWM_SHARED __declspec(dllexport)
	#else
		#define PWM_SHARED __declspec(dllimport)
	#endif
#else
	#define PWM_SHARED
#endif

#endif