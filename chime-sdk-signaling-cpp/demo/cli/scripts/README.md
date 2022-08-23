# Amazon Chime SDK for C++ signaling client cli run script

This is helper script to run the cli demo.

## Prerequisites
1. You have installed [python3](https://www.python.org/downloads/) (>=3.6).

2. Install [requests](https://requests.readthedocs.io/en/latest/)

3. You have deployed [JS Serverless Demo](https://github.com/aws/amazon-chime-sdk-js/tree/main/demos/serverless)

## Running the script
```
python3 run_cli.py -m "<meeting name>" -a "<attendee name>" -f "path/to/pcm/file" -u https://xxxxxxx.execute-api.us-east-1.amazonaws.com/Prod/
# ex)
# python3 scripts/run_cli.py -m "new-meeting" -a "new-attendee" -f "./media_in/pcm_file.pcm" -u https://xxxxxxx.execute-api.us-east-1.amazonaws.com/Prod/
```


## Help
Running with `--help` should show help menu 
```
./scripts/run_cli.py --help
```