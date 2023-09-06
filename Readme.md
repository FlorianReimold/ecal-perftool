# ecal-perftool

The ecal-perftool is a simple application to estimate the performance of eCAL pub-sub connections using dummy-data being published a at a constant frequency.

## Usage

```
Usage:
  ecal_perftool pub <topic_name> <frequency_hz> <payload_size_bytes>
or:
  ecal_perftool sub <topic_name> [callback_delay_ms]
```