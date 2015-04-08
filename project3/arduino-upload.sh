code_file=$1
port=$2

if [[ -z $code_file ]]; then
  echo "Missing argument: file to upload"
  exit 1
fi

if [[ -z $port ]]; then
  port=/dev/ttyACM0
fi

/usr/share/arduino/hardware/tools/avrdude \
-C/usr/share/arduino/hardware/tools/avrdude.conf \
-patmega2560 -cwiring -P$port \
-b115200 -D -V \
-Uflash:w:$code_file:i
