# SDD Setup

All of the oorail controller data is stored in /opt/oorail.
By default this is normally stored on the SD card (or root partition).

If you want to store this on a separate drive (for Raspberry Pi) or a different partition you can do this.

After you run the setup.sh script, you need to create a new mount point for /opt/oorail, to do this:


### Identify the disk device name

For our example, we have installed a San Disk 1TB SDD drive in our raspberry pi 4 setup:

```
pi@raspberrypi:/tmp $ dmesg | grep sd
```

The command dmesg outputs the most recent kernel messages, we are piping it through the grep command to look for sd.

```
[    1.531235] sd 0:0:0:0: [sda] 1953529856 512-byte logical blocks: (1.00 TB/932 GiB)
[    1.534085] sd 0:0:0:0: [sda] Write Protect is off
[    1.536810] sd 0:0:0:0: [sda] Mode Sense: 43 00 00 00
[    1.537158] sd 0:0:0:0: [sda] Write cache: enabled, read cache: enabled, doesn't support DPO or FUA
[    1.540623] sd 0:0:0:0: [sda] Optimal transfer size 33553920 bytes
[    1.548324] sd 0:0:0:0: [sda] Attached SCSI disk
```

This is the output we are interested in, we can identify the drive is our 1.00 TB drive, and we can tell from the [sda] in brackets that its
device /dev/sda. So we can now partition it with the script in our scripts/ directory called makedisk.sh

```
./makedisk.sh sda
```



