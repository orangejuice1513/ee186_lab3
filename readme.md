# EE186 Lab 3

## Part 2: Written Deliverables

### 1. I2C addresses

The seven-bit device addresses are:
- Accelerometer: `0011001b`
- Magnetometer: `0011110b`

where `b` is the read/write bit. (pg.40)

### 2. Sub-address (SUB)

The sub-address identifies an internal register within the sensor. The I²C address selects the device on the bus, while the sub-address selects which register within that device to read or write.

### 3. Enabling acceleration measurements

The accelerometer's `CTRL_REG1_A` register address is
`0x20` (`00100000`). (pg. 47)

We write `0x57` (`01010111` in binary):

| Bits | Field | Value | Function |
| --- | --- | --- | --- |
| 7–4 | ODR | `0101` | 100 Hz output data rate |
| 3 | LPen | `0` | Low-power mode disabled |
| 2–0 | Zen, Yen, Xen | `111` | All three axes enabled |

### 4. Acceleration data sub-addresses
(pg. 47)

Each axis measurement occupies two registers: a low byte and a high byte.

| Axis | Register | 8-bit binary sub-address | Hex |
| --- | --- | --- | --- |
| X | OUT_X_L_A | `00101000` | `0x28` |
| X | OUT_X_H_A | `00101001` | `0x29` |
| Y | OUT_Y_L_A | `00101010` | `0x2A` |
| Y | OUT_Y_H_A | `00101011` | `0x2B` |
| Z | OUT_Z_L_A | `00101100` | `0x2C` |
| Z | OUT_Z_H_A | `00101101` | `0x2D` |

In the code, a six-byte burst read requests all three axes. The transmitted starting SUB is `0xA8`: register address `0x28` with bit 7 set to enable automatic address incrementing.

