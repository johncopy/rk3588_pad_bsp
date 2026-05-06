#!/bin/bash

LOGTAG="set-default-audio-device"

# 等待声卡设备就绪
get_hook_status() {
  busctl --system call ght.platform.dock /ght/platform/dock ght.platform.dock GetHookStatus 2>/dev/null | awk '{print $NF}'
}
for i in {1..100}; do
  pactl load-module module-alsa-source device=softrec
  if pactl list sources | grep -q "alsa_input.softrec"; then
    echo "开始设置平板自身音频通道"	  
    pactl set-default-source alsa_input.softrec
    pactl unload-module module-echo-cancel
    pactl load-module module-echo-cancel \
        source_name=internal_mic \
        source_master=alsa_input.softrec \
        aec_method=webrtc \
        aec_args="analog_gain_control=0"
    pactl set-default-source internal_mic
    pactl set-default-sink alsa_output.platform-es8388-sound.stereo-fallback
    echo "结束设置平板自身音频通道"	  
    break
  fi
  sleep 1
  logger -t "$LOGTAG" ">>>>wait alsa_input.softrec $i>>>>"
done

for i in {1..100}; do
  if pactl list sources | grep -q "alsa_input.usb-Bothlent_Bothlent_UAC_Dongle_88156088-00.multichannel-input"; then
    echo "设置旧底座麦克风阵列回声消除"	  
    pactl load-module module-echo-cancel \
        source_name=micarray_mic \
        source_master=alsa_input.usb-Bothlent_Bothlent_UAC_Dongle_88156088-00.multichannel-input \
	sink_name="speaker"  \
        sink_master="alsa_output.usb-C-Media_Electronics_Inc._USB_Audio_Device-00.analog-stereo" 
        aec_method=webrtc \
        #aec_args="analog_gain_control=0"
        aec_args="analog_gain_control=0,beamforming=1,mobile=1,drift_compensation=1,extended_filter=1,intelligibility_enhancer=1,noise_suppression=1,voice_detection=1,high_pass_filter=1" 
    pactl set-default-source micarray_mic
    pactl set-default-sink alsa_output.usb-C-Media_Electronics_Inc._USB_Audio_Device-00.analog-stereo
    break
  else
    echo "设置新麦克风阵列回声消除"	  
    pactl load-module module-echo-cancel \
        source_name=micarray_mic \
        source_master=alsa_input.usb-SEEED_ReSpeaker_4_Mic_Array__UAC1.0_-00.multichannel-input \
        sink_name="speaker"  \
        sink_master="alsa_output.usb-C-Media_Electronics_Inc._USB_Audio_Device-00.analog-stereo" \
        aec_method=webrtc \
        aec_args="analog_gain_control=0,beamforming=1,mobile=1,drift_compensation=1,extended_filter=1,intelligibility_enhancer=1,noise_suppression=1,voice_detection=1,high_pass_filter=1" 
#	channels=1 \
#	rate=48000 

    pactl set-default-source micarray_mic
    pactl set-default-sink speaker
    break
  fi

  sleep 1
  if pactl list sources | grep -q "alsa_input.usb-Bothlent_Bothlent_UAC_Dongle_88156088-00.multichannel-input"; then
    logger -t "$LOGTAG" ">>>>wait alsa_input.usb-Bothlent_Bothlent_UAC_Dongle_88156088-00.multichannel-input $i>>>>"
  else 
    logger -t "$LOGTAG" ">>>>wait alsa_input.usb-SEEED_ReSpeaker_4_Mic_Array__UAC1.0_-00.multichannel-input $i>>>>"
  fi 
done

for i in {1..100}; do
  if pactl list sources | grep -q "alsa_input.HANDSET_Speakers.mono-fallback"; then
    echo "设置手柄回声消除"	  
    pactl load-module module-echo-cancel \
      source_name=handset_mic \
      source_master=alsa_input.HANDSET_Speakers.mono-fallback \
      aec_method=webrtc \
      aec_args="analog_gain_control=0"
    break
  fi
  sleep 1
  logger -t "$LOGTAG" ">>>>wait alsa_input.usb-C-Media_Electronics_Inc._USB_Audio_Device-00.mono-fallback $i>>>>"
done

# 根据当前摘挂机状态选择默认录音源（若查询失败，保持已设置的默认）
HOOK_STATUS="$(get_hook_status)"
case "$HOOK_STATUS" in
  1)
    pactl set-default-source handset_mic
    pactl set-default-sink   alsa_output.HANDSET_Speakers.analog-stere
    logger -t "$LOGTAG" "HookState=1(摘机): 默认录音源 -> handset_mic"
    ;;
  0)
    if pactl list short sources | awk '{print $2}' | grep -qx micarray_mic; then
      pactl set-default-source micarray_mic
      pactl set-default-sink speaker
      #pactl set-source-volume micarray_mic 80%
      #pactl set-sink-volume   speaker 80%
      logger -t "$LOGTAG" "HookState=0(挂机): 默认录音源 -> micarray_mic"
    else
      pactl set-default-source internal_mic
      logger -t "$LOGTAG" "HookState=0(挂机): 默认录音源 -> internal_mic(备用)"
    fi
    pactl set-default-sink alsa_output.usb-C-Media_Electronics_Inc._USB_Audio_Device-00.analog-stereo
    ;;
  *)
    logger -t "$LOGTAG" "HookState 未知或不可用($HOOK_STATUS): 保持现有默认录音源"
    ;;
esac

exit 0
