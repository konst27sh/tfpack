#include "run_CLIcmd.h"
#include "state_handler.h"
#include "settings_module.h"
#include "net_utils.h"
#include "version.h"

int main(int argc, char **argv)
{
    error_code_t errorCode = ERR_OK;
    openlog("tf_dm", 0, LOG_USER);
    uint8_t run_flag = 0;
    if (argc >=2)
    {
        for(int optid=1; optid < argc; optid++)
        {
            if (argv[optid][0] == '-')
            {
                switch (argv[optid][1])
                {
                    case 'h':
                    {
                        printf("help\n");
                        printf("v - Print Version\n");
                        printf("d - Print Debug Info\n");
                        printf("i - Set interface to bind\n");
                    }
                    return 0;

                    case 'v':
                    {
                        printf("Version %s\n", VERSION);
                    }
                    return 0;

                    case 'd':
                    {
                        printf("debug on\n");
                        setMainTestFlag(1);
                    }
                    break;

                    case 'i':
                    {
                        run_flag = 1;
                        set_DevName(argv[2]);
                    }
                    break;

                    default:
                        printf("help:  -i <devName>" );
                        return 101;
                }
            }
        }
    }

    if (run_flag)
    {
        syslog(LOG_INFO, "t_fortis Device Manager - Version %s - dev name: %s\n", VERSION, argv[2]);
        closelog();
        sleep(15);
        errorCode = client_app();
    }
    else
    {
        syslog(LOG_INFO, "-i <devName> is necessary\n");
        closelog();
    }
    return errorCode;
}

