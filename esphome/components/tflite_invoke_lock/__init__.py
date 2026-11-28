"""One lock for every TensorFlow Lite Micro invocation in the firmware.

The optimized ESP-NN kernels keep their scratch buffers in globals, so two
interpreters must not invoke at the same time. Components that invoke one load
this component and hold ``InvokeLock`` around the call.
"""

import esphome.config_validation as cv
from esphome.types import ConfigType

CODEOWNERS = ["@nliaudat"]
DOMAIN = "tflite_invoke_lock"

# Header only: the components that invoke a model load it.
CONFIG_SCHEMA = cv.Schema({})


async def to_code(config: ConfigType) -> None:
    """Nothing to generate: the lock is a header."""
