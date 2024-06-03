#!/usr/bin/env python3
# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

import sys
assert sys.version_info >= (3, 6), 'This script requires Python 3.6 or greater'

import argparse
import requests
import os
from os import path
import subprocess

parser = argparse.ArgumentParser(description='Run script to run the built cli for Amazon Chime SDK for C++ signaling client')
# required
parser.add_argument('-m', '--meeting-name', dest='mname', help='meeting name', required=True)
parser.add_argument('-a', '--attendee-name', dest='aname', help='attendee name', required=True)
parser.add_argument('-u', '--url', dest='murl', help='meeting url', required=True)

# optional
parser.add_argument('-r', '--region', dest='region', help='region', required=False, default="us-east-1")
parser.add_argument('-f', '--send-audio-file-name', dest='send_audio_file_name', help='48k stereo pcm file to send audio',
                    required=False, default="")
parser.add_argument('-l', '--log-level', dest='log_level',
                    help="log level, choose from verbose, debug, info, warning and error", required=False,
                    default="error")

args = parser.parse_args()

MEETING_URL = args.murl
REGION = args.region
mname = args.mname
aname = args.aname

request_url = f"{MEETING_URL}join?title={mname}&name={aname}&region={REGION}"
print(f"Making requests to {request_url}")
resp = requests.post(request_url)

if resp.status_code != 200:
    print(f'Unable to process received status code {resp.status_code}')
    exit()

attendee_id = resp.json()['JoinInfo']['Attendee']['Attendee']['AttendeeId']
audio_host_url = resp.json()['JoinInfo']['Meeting']['Meeting']['MediaPlacement']['AudioHostUrl']
external_meeting_id = resp.json()['JoinInfo']['Meeting']['Meeting']['ExternalMeetingId']
external_user_id = resp.json()['JoinInfo']['Attendee']['Attendee']['ExternalUserId']
join_token = resp.json()['JoinInfo']['Attendee']['Attendee']['JoinToken']
meeting_id = resp.json()['JoinInfo']['Meeting']['Meeting']['MeetingId']
signaling_url = resp.json()['JoinInfo']['Meeting']['Meeting']['MediaPlacement']['SignalingUrl']

print("Executing my_cli...")
dir_path = os.path.dirname(os.path.abspath(__file__))

executable_name = 'my_cli'

file_path = path.join(dir_path, '..', 'build', executable_name)
print(file_path)

try:
    os.mkdir("media_out")
except OSError:
    pass # Directory already exists

subprocess.check_call([file_path, '--attendee_id', attendee_id,
                                  '--audio_host_url', audio_host_url,
                                  '--external_meeting_id', external_meeting_id,
                                  '--external_user_id', external_user_id,
                                  '--join_token', join_token,
                                  '--log_level', args.log_level,
                                  '--meeting_id', meeting_id,
                                  '--signaling_url', signaling_url,
                                  '--send_audio_file_name', args.send_audio_file_name])
