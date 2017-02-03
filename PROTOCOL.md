# Protocol

Here are some sloppy notes on the 2.4GHz protocol. Borrows from [Henryk's work](https://hackaday.io/project/5888-reverse-engineering-the-milight-on-air-protocol).

## Structure

As far as I can tell, packets are:

* Unidirectional. Bulbs never send packets.
* Always 7 bytes.
* Not fully byte-oriented. One field uses fractional bytes.

The packets are structured as follows:


| Field           | Length  | Notes                                                                                     |
|-----------------|---------|-------------------------------------------------------------------------------------------|
| Request Type    | 1 byte  | One of 0xB0 or 0xB8. Seems to be ignored.                                                                     |
| Device ID       | 2 bytes |                                                                                           |
| Color           | 1 byte  | Maps roughly to `((hue + 40)%360)*(255/359.0)`                                              |
| Brightness      | 5 bits  | Values are [0,25] and aren't ordered intuitively. See extended notes for further details. |
| Group ID        | 3 bits  | Should be in [1,4].                                                                       |
| Button ID       | 1 byte  | Should be in [0x0,0x1A]. See extended notes for list.                                     |
| Sequence Number | 1 byte  | Used by receiver to detect duplicates. Should be incremented each time packets are sent.  |

## Extended Notes

### Brightness

Though the field is 5 bits, only values in the range [0,25] are used. Values from least bright to most bright are:

```
[16, 15, ..., 0, 31, ..., 23]
```

To map a value `x` in `[0,100]` to the protocol value, use:

```c++
  // Expect an input value in [0, 100]. Map it down to [0, 25].
  const uint8_t adjustedBrightness = round(brightness * (25 / 100.0));
  
  // The actual protocol uses a bizarre range where min is 16, max is 23:
  // [16, 15, ..., 0, 31, ..., 23]
  const uint8_t packetBrightnessValue = (
    ((31 - adjustedBrightness) + 17) % 32
  );
```

### Button ID

Basically determines what the request does. See a list of values in [the code](https://github.com/sidoh/esp8266_milight_hub/blob/master/lib/MiLight/MiLightClient.h#L16).
