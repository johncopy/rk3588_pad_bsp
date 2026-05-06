#!/bin/bash

LOGTAG="dock-hook-audio-switch"

#HANDSET_SINK="alsa_output.usb-C-Media_Electronics_Inc._USB_Audio_Device-00.analog-stereo"
HANDSET_SINK="alsa_output.HANDSET_Speakers.analog-stereo"
DOCK_SINK="alsa_output.usb-C-Media_Electronics_Inc._USB_Audio_Device-00.analog-stereo"

MICARRAY_SOURCE_RAW="alsa_input.usb-Bothlent_Bothlent_UAC_Dongle_88156088-00.multichannel-input"
MICARRAY_SOURCE_RAW_NEW="alsa_input.usb-SEEED_ReSpeaker_4_Mic_Array__UAC1.0_-00.multichannel-input"
#HANDSET_SOURCE_RAW="alsa_input.usb-C-Media_Electronics_Inc._USB_Audio_Device-00.mono-fallback"
HANDSET_SOURCE_RAW="alsa_input.HANDSET_Speakers.mono-fallback"



MICARRAY_EC_SOURCE="echo_handfree_mic"
HANDSET_EC_SOURCE="echo_handset_mic"
EC_HANDSET_SINK="echo_handset_speaker"
EC_MICARRAY_SINK="echo_handfree_speaker"

log()
{
  logger -t "$LOGTAG" "$@"
}

ensure_ec_source()
{
  # $1: desired echo-cancel source name
  # $2: master raw source
  local ec_source_name="$1"
  local ec_source_master="$2"

  local ec_sink_name="$3"
  local ec_sink_master="$4"

  # If already exists, nothing to do
  #if pactl list short sources | awk '{print $2}' | grep -qx "$ec_source_name"; then
  #  return 0
  #fi
  echo "开始回声消除设置函数"
  # Make sure master source exists
  for i in {1..20}; do
    if pactl list short sources | awk '{print $2}' | grep -qx "$ec_source_master"; then
      break
    fi
    echo "没有 $ec_source_master"
    sleep 0.5
  done

  for i in {1..20}; do
    if pactl list short sinks | awk '{print $2}' | grep -qx "$ec_sink_master"; then
      break
    fi
    echo "没有 $ec_sink_master"
    sleep 0.5
  done

  echo "完成条件判断"
  # Unload existing module-echo-cancel(s) to avoid duplicates
  while pactl list modules | grep -q "module-echo-cancel"; do
    pactl unload-module module-echo-cancel
  done
  
  sleep 0.3
  echo "设置回声消除 source_name:$ec_source_name source_master:$ec_source_master sink_name:$ec_sink_name sink_master:$ec_sink_master"
  # Create echo-cancel source
  pactl load-module module-echo-cancel \
    source_name="$ec_source_name" \
    source_master="$ec_source_master" \
    sink_name="$ec_sink_name" \
    sink_master="$ec_sink_master" \
    aec_method=webrtc \
    aec_args="digital_gain_control=0,beamforming=1,mobile=1,drift_compensation=1,extended_filter=1,intelligibility_enhancer=1,noise_suppression=1,voice_detection=1,high_pass_filter=1" 
 #   channels=1 \
 #   rate=48000 
  echo "完成回声消除设置函数"
  #aec3_suppression_level=3
  #analog_gain_control=1
  #digital_gain_control=1
}

switch_to_dock()
{
  #旧底座
  if pactl list sources | grep -q $MICARRAY_SOURCE_RAW; then
    echo "旧底座"	  
    ensure_ec_source "$MICARRAY_EC_SOURCE" "$MICARRAY_SOURCE_RAW" "$EC_MICARRAY_SINK" "$DOCK_SINK"
  else
    echo "新底座"
    ensure_ec_source "$MICARRAY_EC_SOURCE" "$MICARRAY_SOURCE_RAW_NEW" "$EC_MICARRAY_SINK" "$DOCK_SINK"	  
  fi

  pactl set-default-source "$MICARRAY_EC_SOURCE" || true

  echo "开始等待$EC_MICARRAY_SINK"	  
  # Switch default sink to dock USB
  for i in {1..10}; do
    if pactl list short sinks | awk '{print $2}' | grep -qx "$EC_MICARRAY_SINK"; then
      pactl set-default-sink "$EC_MICARRAY_SINK"
      break
    fi
    sleep 0.5
  done
  echo "完成等待$EC_MICARRAY_SINK"

  pactl set-source-volume  $MICARRAY_EC_SOURCE  100%
  log "HookState=0(挂机): set source=$MICARRAY_EC_SOURCE, sink=$EC_MICARRAY_SINK"
}

switch_to_handset()
{
  ensure_ec_source "$HANDSET_EC_SOURCE" "$HANDSET_SOURCE_RAW" "$EC_HANDSET_SINK"  "$HANDSET_SINK"
  pactl set-default-source "$HANDSET_EC_SOURCE" || true
  for i in {1..10}; do
    if pactl list short sinks | awk '{print $2}' | grep -qx "$EC_HANDSET_SINK"; then
      pactl set-default-sink "$EC_HANDSET_SINK"
      break
    fi
    sleep 0.5
  done
  pactl set-source-volume  $HANDSET_EC_SOURCE  80%
  
  #不开启回声消除，防止摘机从底座喇叭放音
  #pactl set-default-sink   $HANDSET_SINK
  #pactl set-default-source $HANDSET_SOURCE_RAW
  log "HookState=1(摘机): set source=$HANDSET_EC_SOURCE, sink=$EC_HANDSET_SINK"
}

wait_pulseaudio()
{
  for i in {1..60}; do
    if pactl info >/dev/null 2>&1; then
      return 0
    fi
    sleep 0.5
  done
  return 1
}

main()
{
  wait_pulseaudio || log "Warning: PulseAudio not ready, continuing anyway"

  # Start monitor for HookStateChanged on system bus
  stdbuf -oL dbus-monitor --system "interface='ght.platform.dock',member='HookStateChanged'" |
  while IFS= read -r line; do
    case "$line" in
      *"int32 0"*)
        pactl set-sink-mute $DOCK_SINK 1
	echo "挂机"      
        switch_to_dock
	echo "完成挂机处理"      
        pactl set-sink-mute $DOCK_SINK 0
        ;;
      *"int32 1"*)
        pactl set-sink-mute $DOCK_SINK 1
	echo "摘机"
        switch_to_handset
	echo "完成摘机处理"      
	pactl set-sink-mute $DOCK_SINK 0
        ;;
      *)
        ;;
    esac
  done
  echo "脚本退出"
}

main "$@"


