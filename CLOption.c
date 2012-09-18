
#include "CLOption.h"

void 
clShowOptions(CLOption* options, cl_uint count) 
{
    cl_uint size_longer = 0;
    for(cl_uint i = 0; i < count; i++) {
        if(strlen(options[i].short_name) + strlen(options[i].long_name) > size_longer) 
            size_longer = (cl_uint)(strlen(options[i].short_name) + strlen(options[i].long_name));
    }
    for(cl_uint i = 0; i < count; i++) {
        char opt[1024] = { 0 };
        sprintf(opt, "%s, %s", options[i].short_name, options[i].long_name);
        printf("%-*s   %s\n", size_longer + 2, opt, options[i].description);
        
        if(options[i].type == CL_DEVICE_OPTION) {
            cl_uint dev_count = 0;
            CLDeviceInfo* devices = clGetDeviceList(&dev_count);
            printf("%-*s   %s\n", size_longer + 2, "", "Available OpenCL devices");
            for(cl_uint d = 0; d < dev_count; d++) {
                char* dev_type;
                clDeviceTypeToString(devices[d].type, &dev_type);
                printf("%-*s   %d) %s (%s)\n", size_longer + 2, "", d,  devices[d].name, dev_type);
                free(dev_type);
                clFreeDeviceInfo(&devices[d]);
            }
            free(devices);
        }
    }
    char opt[1024] = { 0 };
    sprintf(opt, "%s, %s", "-h", "--help");
    printf("%-*s   %s\n", size_longer + 2, opt, "Displays this list");
}

void 
clParseOptions(CLOption* options, cl_uint option_count, int argc, const char * argv[]) 
{
    // Look for -h or --help
    for(cl_uint i = 1; i < argc; i+=1) {
        const char* opt = argv[i];
        if(strcmp("-h", opt) == 0 || strcmp("--help", opt) == 0) {
            clShowOptions(options, option_count);
            exit(0);
        }
    }
    
    // Look for anything else
    for(cl_uint j = 0; j < option_count; j++) {
        bool found = false;
        if(options[j].type == CL_BOOL_OPTION)
            *((bool*)options[j].value) = false;
            
        for(cl_uint i = 1; i < argc; i+=1) {
            const char* opt = argv[i];
            if(strcmp(options[j].short_name, opt) == 0 ||
               strcmp(options[j].long_name, opt) == 0) {
                found = true;
                switch(options[j].type) {
                    case CL_INT_OPTION:
                    case CL_DEVICE_OPTION:
                        if(argc <= i + 1) {
                            printf("Error parsing command line args (arg %s)\n", argv[i]);
                            return;
                        }
                        *((int*)options[j].value) = atoi(argv[i + 1]);
                        break;
                    case CL_FLOAT_OPTION:
                        if(argc <= i + 1) {
                            printf("Error parsing command line args (arg %s)\n", argv[i]);
                            return;
                        }
                        *((float*)options[j].value) = atof(argv[i + 1]);
                        break;
                    case CL_DOUBLE_OPTION:
                        if(argc <= i + 1) {
                            printf("Error parsing command line args (arg %s)\n", argv[i]);
                            return;
                        }
                        *((double*)options[j].value) = atof(argv[i + 1]);
                        break;
                    case CL_STRING_OPTION:
                        if(argc <= i + 1) {
                            printf("Error parsing command line args (arg %s)\n", argv[i]);
                            return;
                        }
                        *((char**)options[j].value) = malloc((strlen(argv[i + 1]) + 1) * sizeof(char));
                        memset(*((char**)options[j].value), 0, (strlen(argv[i + 1]) + 1) * sizeof(char));
                        strcpy(*((char**)options[j].value), argv[i + 1]);
                        break;
                    case CL_BOOL_OPTION:
                        *((bool*)options[j].value) = true;
                        break;
                    default:
                        break;
                }
            }
        }
    }
}