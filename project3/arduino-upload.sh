code_file=$1

if [[ -z $code_file ]]; then
  echo "Missing argument: file to upload"
  exit 1
fi

/usr/share/arduino/hardware/tools/avrdude \
-C/usr/share/arduino/hardware/tools/avrdude.conf \
-v -v -v -v -patmega2560 -cwiring -P/dev/ttyACM0 \
-b115200 -D -V \
-Uflash:w:$code_file:i
