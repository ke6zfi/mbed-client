#include <string.h>
#include <stdlib.h> 
#include <stdarg.h>

//libServices
#include "ns_types.h"
#include "ns_cmdline.h"
#include "cmd_lwm2m.h"
#include "lwm2mtest.h"

#define CMD_MAN_LWM2M_CLIENT        "lwm2m-client <cmd> [options]                   LWM2M mbed Client API\n"\
                                    "setup <p> [options]                            Set up the LWM2M Interface\n"\
                                    "<p>:\n"\
                                    "   Options for setup command\n"\
                                    "   --endpoint <name>           Endpoint Name\n"\
                                    "   --type  <name>              Resource Type\n"\
                                    "   --lifetime <n>              Lifetime in seconds, default is -1 means not included\n"\
                                    "   --port <n>                  Listen port, default is 5683\n"\
                                    "   --domain <name>             Domain for mbed Device Server, default is empty\n"\
                                    "   --binding_mode <n>          Binding Mode, NOT_SET = 0|UDP = 1(default)|QUEUE = 2|SMS = 4\n"\
                                    "   --network_interface <n>     Network Interface, Uninitialized = 0 ,LwIP_IPv4 = 1(default), LwIP_IPv6 = 2, Nanostack_IPv6 = 4\n"\
                                    "bootstrap_object <p> [options]\n"\
                                    "<p>:\n"\
                                    "   Options for bootstrap object command\n"\
                                    "   --address <name>            Bootstrap server address, format is coap://192.168.0.1:5683\n"\
                                    "bootstrap                      Issues Bootstrap command\n"\
                                    "object <p> [options]\n"\
                                    "<p>:\n"\
                                    "   Options for custom object\n"\
                                    "   --name <name>       Object name\n"\
                                    "   --new_instance <n>  If you need new instance 0=false(default), 1=true\n"\
                                    "static_resource <p> [options]\n"\
                                    "<p>:\n"\
                                    "   Options for static resource\n"\
                                    "   --object_instance <n> Instance Id of the object this resource is associated with, default is 0\n"\
                                    "   --name <name>       Resource name\n"\
                                    "   --value <name>      Resource value\n"\
                                    "   --value_type <n>    Value Type String=0, Integer=1\n"\
                                    "   --multiple_instance <n> Supports multiple instances, false=0(default), true=1\n"\
                                    "dynamic_resource <p> [options]\n"\
                                    "<p>:\n"\
                                    "   Options for dynamic resource\n"\
                                    "   --object_instance <n> Instance Id of the object this resource is associated with, default is 0\n"\
                                    "   --name <name>       Resource name\n"\
                                    "   --observable <n>    Resource is observable false=0(default), true=1\n"\
                                    "   --multiple_instance <n> Supports multiple instances, false=0(default), true=1\n"\
                                    "device <p> [options]\n"\
                                    "<p>:\n"\
                                    "   Options for device object \n"\
                                    "   --manufacturer <name>       Manufacturer name\n"\
                                    "   --model_number <name>       Model number\n"\
                                    "   --serial_number <name>      Serial number\n"\
                                    "   --device_type <name>        Device Type\n"\
                                    "   --hardware_version <name>   Hardware version\n"\
                                    "   --software_version <name>   Software version\n"\
                                    "   --firmware_version <name>   Firmware version\n"\
                                    "   --available_power_sources <n> Number of available power sources\n"\
                                    "   --power_source_voltage <n>   Power source voltage\n"\
                                    "   --power_source_current <n>   Power source current\n"\
                                    "   --battery_level <n>        Battery level\n"\
                                    "   --battery_status <n>   Battery status\n"\
                                    "   --memory_free <n>   Free memory, in bytes\n"\
                                    "   --memory_total <n>   Free memory in bytes\n"\
                                    "   --error_code <n>   Error Code\n"\
                                    "   --current_time <n>   Current Time, EPOCH format\n"\
                                    "   --utc_offset <name>   UTC Format\n"\
                                    "   --timezone <name>   Time zone \n"\
                                    "register_object <p> [options]\n"\
                                    "<p>:\n"\
                                    "   Options for register command\n"\
                                    "   --address <name>            mbed Device Server address, format is coap://192.168.0.1:5683\n"\
                                    "register                       Issues Register command\n"\
                                    "update-register <p> [options]  Issues Update registration command\n"\
                                    "<p>:\n"\
                                    "   Options for update-register command\n"\
                                    "   --lifetime <n>              Lifetime value in seconds\n"\
                                    "unregister                     Issues Un-register command\n"

#define EXIT_MANUAL         "exit :closes the application\n"


int lwm2m_client_command(int argc, char *argv[]);
int lwm2m_client_setup_command(int argc, char *argv[]);
int lwm2m_client_device_command(int argc, char *argv[]);
int lwm2m_client_object_command(int argc, char *argv[]);
int lwm2m_client_static_resource_command(int argc, char *argv[]);
int lwm2m_client_dynamic_resource_command(int argc, char *argv[]);
int lwm2m_client_bootstrap_object_command(int argc, char *argv[]);
int lwm2m_client_bootstrap_command();
int lwm2m_client_register_object_command(int argc, char *argv[]);
int lwm2m_client_register_command();
int lwm2m_client_update_register_command(int argc, char *argv[]);
int lwm2m_client_unregister_command();
int exit_command(int argc, char *argv[]);

void  lwm2m_command_init(void)
{
  cmd_add("lwm2m-client", lwm2m_client_command, "LWM2M Client specific command", CMD_MAN_LWM2M_CLIENT);
  cmd_alias_add("lwm2m-client-test-setup", "lwm2m-client setup --endpoint lwm2m-endpoint --type test --lifetime 3600");
  cmd_alias_add("lwm2m-client-test-device", "lwm2m-client device --manufacturer ARM --model_number 2015 --serial_number 12345");
  cmd_alias_add("lwm2m-client-test-bootstrap-object", "lwm2m-client bootstrap_object --address coap://10.45.3.10:5693");
  cmd_alias_add("lwm2m-client-test-register-object", "lwm2m-client register_object --address coap://10.45.3.10:5683");
  cmd_add("exit", exit_command, "exit command", EXIT_MANUAL);
}
char *test_mem_block = 0;
int   test_timer;
M2MLWClient lwm2m_client;

int lwm2m_client_command(int argc, char *argv[])
{
    if( strcmp(argv[1], "setup") == 0 )
    {
      return lwm2m_client_setup_command(argc, argv);
    }
    else if( strcmp(argv[1], "bootstrap_object") == 0 )
    {
      return lwm2m_client_bootstrap_object_command(argc, argv);
    }
    else if( strcmp(argv[1], "bootstrap") == 0 )
    {
      return lwm2m_client_bootstrap_command();
    }
    else if( strcmp(argv[1], "device") == 0 )
    {
      return lwm2m_client_device_command(argc, argv);
    }
    else if( strcmp(argv[1], "object") == 0 )
    {
      return lwm2m_client_object_command(argc, argv);
    }
    else if( strcmp(argv[1], "static_resource") == 0 )
    {
      return lwm2m_client_static_resource_command(argc, argv);
    }
    else if( strcmp(argv[1], "dynamic_resource") == 0 )
    {
      return lwm2m_client_dynamic_resource_command(argc, argv);
    }
    else if( strcmp(argv[1], "register_object") == 0 )
    {
      return lwm2m_client_register_object_command(argc, argv);
    }
    else if( strcmp(argv[1], "register") == 0 )
    {
      return lwm2m_client_register_command();
    }
    else if( strcmp(argv[1], "update-register") == 0 )
    {
      return lwm2m_client_update_register_command(argc, argv);
    }
    else if( strcmp(argv[1], "unregister") == 0 )
    {
      return lwm2m_client_unregister_command();
    }
    //:TODO what another commands should be there ?
    return CMDLINE_RETCODE_COMMAND_NOT_IMPLEMENTED;
}

int lwm2m_client_setup_command(int argc, char *argv[])
{
    char *endpoint = 0;
    char *type = 0;
    int lifetime = -1;
    int32_t port = 8000;
    char *domain = 0;
    int32_t binding_mode = 1;
    int32_t network_interface = 1;
    cmd_parameter_val(argc, argv, "--endpoint", &endpoint);
    cmd_parameter_val(argc, argv, "--type", &type);
    cmd_parameter_int(argc, argv, "--lifetime", &lifetime);
    cmd_parameter_int(argc, argv, "--port", &port);
    cmd_parameter_val(argc, argv, "--domain", &domain);
    cmd_parameter_int(argc, argv, "--binding_mode", &binding_mode);
    cmd_parameter_int(argc, argv, "--network_interface", &network_interface);

    if(lwm2m_client.create_interface(endpoint,type,lifetime,port,
                                domain,binding_mode,network_interface)){
        return CMDLINE_RETCODE_SUCCESS;
    }
    return CMDLINE_RETCODE_INVALID_PARAMETERS;
}

int lwm2m_client_device_command(int argc, char *argv[])
{
    int return_code = CMDLINE_RETCODE_SUCCESS;// CMDLINE_RETCODE_INVALID_PARAMETERS;
    char *manufacturer = 0;
    char *model_number = 0;
    char *serial_number = 0;
    char *device_type = 0;
    char *hardware_version = 0;
    char *software_version = 0;
    char *firmware_version = 0;
    char *current_time = 0;
    char *utc_offset = 0;
    char *timezone = 0;
    int32_t  available_power_sources = 0;
    int32_t  power_source_voltage = 0;
    int32_t  power_source_current = 0;
    int32_t  battery_status = 0;
    int32_t  battery_level = 0;
    int32_t  memory_free = 0;
    int32_t  memory_total = 0;
    int32_t  error_code = 0;

    if(cmd_parameter_val(argc, argv, "--manufacturer", &manufacturer)) {
        if(!lwm2m_client.create_device_object(M2MDevice::Manufacturer,
                                          manufacturer)) {
            return CMDLINE_RETCODE_INVALID_PARAMETERS;
        }
    }
    if(cmd_parameter_val(argc, argv, "--model_number", &model_number)) {
        if(!lwm2m_client.create_device_object(M2MDevice::ModelNumber,
                                              model_number)) {
            return CMDLINE_RETCODE_INVALID_PARAMETERS;
        }
    }
    if(cmd_parameter_val(argc, argv, "--serial_number", &serial_number)){
        if(!lwm2m_client.create_device_object(M2MDevice::SerialNumber,
                                                  serial_number)) {
            return CMDLINE_RETCODE_INVALID_PARAMETERS;
        }
    }
    if(cmd_parameter_val(argc, argv, "--device_type", &device_type)){
       if(!lwm2m_client.create_device_object(M2MDevice::DeviceType,
                                             device_type)) {
           return CMDLINE_RETCODE_INVALID_PARAMETERS;
       }
   }
    if(cmd_parameter_val(argc, argv, "--hardware_version", &hardware_version)){
       if(!lwm2m_client.create_device_object(M2MDevice::HardwareVersion,
                                             hardware_version)) {
            return CMDLINE_RETCODE_INVALID_PARAMETERS;
        }
   }
    if(cmd_parameter_val(argc, argv, "--software_version", &software_version)){
       if(!lwm2m_client.create_device_object(M2MDevice::SoftwareVersion,
                                             software_version)) {
            return CMDLINE_RETCODE_INVALID_PARAMETERS;
        }
   }
    if(cmd_parameter_val(argc, argv, "--firmware_version", &firmware_version)){
       if(!lwm2m_client.create_device_object(M2MDevice::FirmwareVersion,
                                             firmware_version)) {
           return CMDLINE_RETCODE_INVALID_PARAMETERS;
        }
   }
    if(cmd_parameter_int(argc, argv, "--available_power_sources", &available_power_sources)){
       if(!lwm2m_client.create_device_object(M2MDevice::AvailablePowerSources,
                                             available_power_sources)) {
            return CMDLINE_RETCODE_INVALID_PARAMETERS;
        }
   }
    if(cmd_parameter_int(argc, argv, "--power_source_voltage", &power_source_voltage)){
       if(!lwm2m_client.create_device_object(M2MDevice::PowerSourceVoltage,
                                             power_source_voltage)) {
            return CMDLINE_RETCODE_INVALID_PARAMETERS;
        }
   }
    if(cmd_parameter_int(argc, argv, "--power_source_current", &power_source_current)){
       if(!lwm2m_client.create_device_object(M2MDevice::PowerSourceCurrent,
                                             power_source_current)) {
            return CMDLINE_RETCODE_INVALID_PARAMETERS;
        }
   }
    if(cmd_parameter_int(argc, argv, "--battery_level", &battery_level)){
       if(!lwm2m_client.create_device_object(M2MDevice::BatteryLevel,
                                             battery_level)) {
            return CMDLINE_RETCODE_INVALID_PARAMETERS;
        }
   }
    if(cmd_parameter_int(argc, argv, "--battery_status", &battery_status)){
       if(!lwm2m_client.create_device_object(M2MDevice::BatteryStatus,
                                             battery_status)) {
            return CMDLINE_RETCODE_INVALID_PARAMETERS;
        }
   }
    if(cmd_parameter_int(argc, argv, "--memory_free", &memory_free)){
       if(!lwm2m_client.create_device_object(M2MDevice::MemoryFree,
                                             memory_free)) {
           return CMDLINE_RETCODE_INVALID_PARAMETERS;
       }
   }
    if(cmd_parameter_int(argc, argv, "--memory_total", &memory_total)){
       if(!lwm2m_client.create_device_object(M2MDevice::MemoryTotal,
                                             memory_total)) {
           return CMDLINE_RETCODE_INVALID_PARAMETERS;
       }
   }
    if(cmd_parameter_int(argc, argv, "--error_code", &error_code)){
       if(!lwm2m_client.create_device_object(M2MDevice::ErrorCode,
                                             error_code)) {
           return CMDLINE_RETCODE_INVALID_PARAMETERS;
       }
   }
    if(cmd_parameter_val(argc, argv, "--current_time", &current_time)){
       if(!lwm2m_client.create_device_object(M2MDevice::CurrentTime,
                                             current_time)) {
           return CMDLINE_RETCODE_INVALID_PARAMETERS;
       }
   }
    if(cmd_parameter_val(argc, argv, "--utc_offset", &utc_offset)){
       if(!lwm2m_client.create_device_object(M2MDevice::UTCOffset,
                                             utc_offset)) {
           return CMDLINE_RETCODE_INVALID_PARAMETERS;
       }
   }
    if(cmd_parameter_val(argc, argv, "--timezone", &timezone)){
       if(!lwm2m_client.create_device_object(M2MDevice::Timezone,
                                             timezone)) {
           return CMDLINE_RETCODE_INVALID_PARAMETERS;
       }
   }

   return return_code;
}

int lwm2m_client_object_command(int argc, char *argv[])
{
    int return_code = CMDLINE_RETCODE_SUCCESS;// CMDLINE_RETCODE_INVALID_PARAMETERS;
    char *object_name = 0;
    int32_t new_instance = 0;

    if(cmd_parameter_val(argc, argv, "--name", &object_name)) {
        cmd_parameter_int(argc, argv, "--new_instance", &new_instance);
        if(!lwm2m_client.create_object(object_name,new_instance)) {
            return_code = CMDLINE_RETCODE_INVALID_PARAMETERS;
         }
    }
    return return_code;
}

int lwm2m_client_static_resource_command(int argc, char *argv[])
{
    int return_code = CMDLINE_RETCODE_INVALID_PARAMETERS;
    char *name = 0;
    char *value_string = 0;
    int32_t value_int = 0;
    int32_t value_type = -1;
    int32_t multiple_instance = 0;
    int32_t object_instance = 0;

    cmd_parameter_int(argc, argv, "--multiple_instance", &multiple_instance);
    cmd_parameter_int(argc, argv, "--object_instance", &object_instance);

    if(cmd_parameter_int(argc, argv, "--value_type", &value_type)) {
        if(0 == value_type){
            if(cmd_parameter_val(argc, argv, "--name", &name) &&
               cmd_parameter_val(argc, argv, "--value", &value_string)) {
                if(lwm2m_client.create_static_resource_string(name,value_string,
                                                              multiple_instance,object_instance)) {
                    return_code =  CMDLINE_RETCODE_SUCCESS;
                }
            }
        } else if(1 == value_type){
            if(cmd_parameter_val(argc, argv, "--name", &name) &&
               cmd_parameter_int(argc, argv, "--value", &value_int)) {
                if(lwm2m_client.create_static_resource_int(name,value_int,
                                                           multiple_instance,object_instance)) {
                    return_code =  CMDLINE_RETCODE_SUCCESS;
                }
            }
        }
    }
    return return_code;
}

int lwm2m_client_dynamic_resource_command(int argc, char *argv[])
{
    int return_code = CMDLINE_RETCODE_INVALID_PARAMETERS;
    char *name = 0;
    int32_t multiple_instance = 0;
    int32_t object_instance = 0;
    int32_t observable = 0;

    cmd_parameter_int(argc, argv, "--multiple_instance", &multiple_instance);
    cmd_parameter_int(argc, argv, "--object_instance", &object_instance);
    cmd_parameter_int(argc, argv, "--observable", &observable);

    if(cmd_parameter_val(argc, argv, "--name", &name)) {
        if(lwm2m_client.create_dynamic_resource(name,observable,
                                                multiple_instance,object_instance)) {
            return_code =  CMDLINE_RETCODE_SUCCESS;
        }
    }
    return return_code;
}

int lwm2m_client_bootstrap_object_command(int argc, char *argv[])
{
    int return_code = CMDLINE_RETCODE_FAIL;
    char *address = 0;

    if( cmd_parameter_val(argc, argv, "--address", &address) ){
        if(lwm2m_client.create_bootstrap_object(address)){
            return_code = CMDLINE_RETCODE_SUCCESS;
        }
    } else {
        return_code = CMDLINE_RETCODE_INVALID_PARAMETERS;
    }
    return return_code;
}

int lwm2m_client_bootstrap_command()
{
    int return_code = CMDLINE_RETCODE_FAIL;

    if(lwm2m_client.test_bootstrap()){
        return_code = CMDLINE_RETCODE_EXCUTING_CONTINUE;
    }
    return return_code;
}

int lwm2m_client_register_object_command(int argc, char *argv[])
{
    char *address = 0;
    cmd_parameter_val(argc, argv, "--address", &address);

   if(lwm2m_client.create_register_object(address)) {
        return CMDLINE_RETCODE_SUCCESS;
    }
    return CMDLINE_RETCODE_INVALID_PARAMETERS;
}

int lwm2m_client_register_command()
{
    if(lwm2m_client.test_register()) {
        return CMDLINE_RETCODE_EXCUTING_CONTINUE;
    }
    return CMDLINE_RETCODE_INVALID_PARAMETERS;
}

int lwm2m_client_update_register_command(int argc, char *argv[])
{
    int ret_code = CMDLINE_RETCODE_INVALID_PARAMETERS;
    int32_t lifetime = -1;

    cmd_parameter_int(argc, argv, "--lifetime", &lifetime);

    if(lifetime > 0) {
        if(lwm2m_client.test_update_register(lifetime)) {
            ret_code = CMDLINE_RETCODE_SUCCESS;
        }
    }
    return ret_code;
}

int lwm2m_client_unregister_command()
{
    if(lwm2m_client.test_unregister()) {
        return CMDLINE_RETCODE_EXCUTING_CONTINUE;
    }
    return CMDLINE_RETCODE_INVALID_PARAMETERS;
}

int exit_command(int argc, char *argv[])
{
    exit(1);
}