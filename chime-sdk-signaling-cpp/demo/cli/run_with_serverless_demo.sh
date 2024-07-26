#!/bin/bash

# Default values. Feel free to override these instead of providing as arguments every run.
MEETING_URL=""
REGION="us-east-1"
LOG_LEVEL="error"

usage() {
  echo "Usage: $0 -m <meeting> -a <attendee> [options]"
  echo "  -m, --meeting <meeting name>        Required. Meeting name."
  echo "  -a, --attendee <attendee name>      Required. Attendee name."
  echo "  -u, --url <meeting url>                  Meeting URL. Default: $MEETING_URL"
  echo "  -r, --region <region>                    AWS region. Default: $REGION"
  echo "  -l, --log-level <level>                  Log level (verbose, debug, info, warning, error). Default: $LOG_LEVEL"
  echo "  -h, --help                               Display this help and exit."
}

while [[ "$#" -gt 0 ]]; do
  case $1 in
    -m|--meeting) meeting="$2"; shift ;;
    -a|--attendee) attendee="$2"; shift ;;
    -u|--url) MEETING_URL="$2"; shift ;;
    -r|--region) REGION="$2"; shift ;;
    -l|--log-level) LOG_LEVEL="$2"; shift ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown parameter passed: $1"; usage; exit 1 ;;
  esac
  shift
done

if [[ -z "$meeting" || -z "$attendee" ]]; then
  echo "Meeting name and attendee name are required."
  usage
  exit 1
fi

# Perform the request
request_url="${MEETING_URL}join?title=${meeting}&name=${attendee}&region=${REGION}"
echo "Making requests to $request_url"
response=$(curl -s -X POST "$request_url")

# Extract JSON data
attendee_id=$(echo "$response" | jq -r '.JoinInfo.Attendee.Attendee.AttendeeId')
audio_host_url=$(echo "$response" | jq -r '.JoinInfo.Meeting.Meeting.MediaPlacement.AudioHostUrl')
external_meeting_id=$(echo "$response" | jq -r '.JoinInfo.Meeting.Meeting.ExternalMeetingId')
external_user_id=$(echo "$response" | jq -r '.JoinInfo.Attendee.Attendee.ExternalUserId')
join_token=$(echo "$response" | jq -r '.JoinInfo.Attendee.Attendee.JoinToken')
meeting_id=$(echo "$response" | jq -r '.JoinInfo.Meeting.Meeting.MeetingId')
signaling_url=$(echo "$response" | jq -r '.JoinInfo.Meeting.Meeting.MediaPlacement.SignalingUrl')

# Prepare for execution
dir_path=$(dirname "$(realpath "$0")")
executable_name='my_cli'
file_path="${dir_path}/build/${executable_name}"

# Execute the CLI with parameters
"$file_path" --attendee_id "$attendee_id" \
             --audio_host_url "$audio_host_url" \
             --external_meeting_id "$external_meeting_id" \
             --external_user_id "$external_user_id" \
             --join_token "$join_token" \
             --log_level "$LOG_LEVEL" \
             --meeting_id "$meeting_id" \
             --signaling_url "$signaling_url"