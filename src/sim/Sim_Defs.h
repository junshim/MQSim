#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include<cstdint>
#include<string>
#include<iostream>

#define NOT_GC_WL 0
#define YES_GC_WL 1

#define NEW_SUBMIT true
#define SOFT_ISOLATION true

#define MLC_TLC_GC_THRESHOLD (false)

#define SET_PRECONDION_TO_UNIFORM_RND (true) // Trace-base WL  
#define MULTI_PROCESS_IO_GEN  (false)
#define ACT_SIMULATION        (false)
#define ACT_STRESS_CHECK_ENABLE   (false)

#define ERS_CANCEL_ENABLE         (false) //true 
#define ERS_CANCEL_TO_ENABLE      (false)  //true
#define ERS_DEFER_SUS_ENABLE      (false) //true
#define ERS_IDEAL_SUS_ENABLE      (false)//false //ERASE°¡ suspendµÇ´õ¶óµµ ÆÐ³ÎÆ¼°¡ ¾øÀ½. 
#define ERS_SUS_FAST12            (false)//false
#define ADAPTIVE_RPS_SCH_ENABLE   (false)

#define PGM_SUS_ENABLE        (false) //false

#define INCREMENTAL_GC (true) //user write°ü·Ã. °íÁ¤
#define SCHEDULE_NORM	(true) //user read°ü·Ã
#define ADAPTIVE_GCUSR (false) //doodu
#define SCHEDULE_ATC (false) //만약 프로그램이 계속 죽어=> 이놈을 true로 하고 SCHEDULE_NORM을 false


#define MAX_SUBBLOCK_ERS_CNT 3//if 2-> can erase total 3 sub-blocks from 1 block. 7 is max
#define INVALID_GRANU 10 
#define THRESHOLD_INVALID 0 //6

#define REMOVE_LATENCY 1 //¸¸¾à ÀÌh°Ô 1ÀÌ¶ó¸é start_command_execution¿¡¼­ subblock eraseÀÇ °æ¿ì erase latency¸¦ dieBKE->time°ü·Ã¿¡ ´õÇÏÁö ¾ÊÀ¸¹Ç·Î service_read_transaction¿¡¼­ Ãß°¡ÀûÀÎ subblk erase¶§¹®¿¡ read°¡ ¼­ºñ½º ¸ø¹Þ´Â°æ¿ì´Â ¾øÀ½
#define REMOVE_LATENCY_SEND 1 //send_command_to_chip(), Execute_simulator_event()
#define REMOVE_LATENCY_TMP 1 //PRIMARY_ERS°ü·Ã. ¸¸ÀÏ preemptible¿Í PE SUSPEND/RESUMEÀ¸·Î ÀÎÇÑ free block °¨¼Ò¸¦ º¼ ½Ã, 0À¸·Î ¼¼ÆÃ

#define REMOVE_LAT_GC_W 0 
#define DEVISION_FACTOR 1.5 // GC write/read overhead factor
//double DEVISION_FACTOR = 1.5; // GC write/read overhead factor

#define NO_MULTIPLANE_COMMAND 1
#define RL_sarsa 1
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define FREE_ALIGN 1
#define STRICT_ALIGN 0
#define BLOCK_POOL_GC_HARD_THRESHOLD_ADJUST 0.01//0.6//0.3//adjust urgent GC trigger threshold //0.1. = 0.01*block_pool_gc_hard_threshold = (unsigned int)(gc_hard_threshold * (double)block_no_per_plane);
//0.01(BLOCK_POOL_GC_HARD_THRESHOLD_ADJUST) * 0.05 * block_no_per_plane > block pool -> urgent GC

typedef uint64_t sim_time_type;
typedef uint16_t stream_id_type;
typedef sim_time_type data_timestamp_type;

#define INVALID_TIME 18446744073709551615ULL 
#define T0 0
#define INVALID_TIME_STAMP 18446744073709551615ULL
#define MAXIMUM_TIME 18446744073709551615ULL
#define ONE_SECOND 1000000000
typedef std::string sim_object_id_type;

#define CurrentTimeStamp Simulator->Time()
#define PRINT_ERROR(MSG) {\
							std::cerr << "ERROR:" ;\
							std::cerr << MSG << std::endl; \
							std::cin.get();\
							exit(1);\
						 }
#define PRINT_ERROR_NO_CIN(MSG) {\
							std::cerr << "ERROR:" ;\
							std::cerr << MSG << std::endl; \
							exit(1);\
						 }

#define PRINT_MESSAGE(M) std::cout << M << std::endl;
#define DEBUG(M) //std::cout<<M<<std::endl;
#define DEBUG2(M) //std::cout<<M<<std::endl;

#define DCACHE_DEBUG(M) (std::cout << "D-CACHE_DEBUG : " << M << std::endl)

#define SIM_TIME_TO_MICROSECONDS_COEFF 1000
#define SIM_TIME_TO_SECONDS_COEFF 1000000000
#define SIM_TIME_TO_MS_COEFF 1000000

//#define INCREMENTAL_GC (false) //true
#define BACK_PRESSUER_BUF_MAG (128)
#define ERS_SUS_DIST_COUNT   (2000)

#define ERS_TIME_PER_LOOP (1000000)  //default: 1000000, ÁÙÀÌ¸é Issued_Flash_Suspend_Erase_CMD°¡ ´Ã¾î³ªÁö¸¸ read ¼º´ÉÀº ÁÁ¾ÆÁö°í µüÈ÷ free block¼öµµ º¯µ¿x

#define ACT_SMALL_DIV (2000)
#define ACT_LARGE_DIV  (24)
#define ACT_STRESS_TO (10000000000)


extern sim_time_type gnACTLargeReadCount;
extern sim_time_type gnACTLargeWriteCount;
extern sim_time_type gnACTSmallReadCount;

extern bool READ_PRIO_SCH;

extern size_t gnERSSuspendOffCount;
extern unsigned int ERS_SUS_TO_Time_MS;
extern unsigned int ACT_MAG_COUNT;

extern size_t gnErsSusToCount[];


extern size_t gnERSSusCount;
extern size_t gnDefERSCount;
extern size_t gnTotalERSCount;


#endif // !DEFINITIONS_H
