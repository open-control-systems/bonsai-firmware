#include "control_pipeline.h"

using namespace ocs;
using namespace ocs::app;

extern "C" void app_main(void) {
    ControlPipeline pipeline;
    pipeline.start();
}
