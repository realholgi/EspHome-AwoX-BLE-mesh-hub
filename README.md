# ESPHome component: AwoX BLE mesh (mqtt) hub

A ESPhome component (https://esphome.io/components/external_components.html#git) to create a MQTT hub for your AwoX BLE mesh devices.

You will need your mesh credentials, easiest way to find/get these is to use [this form](https://fsaris.github.io/EspHome-AwoX-BLE-mesh-hub/awoxh-mesh-credentials-tool/) to read them from your AwoX Cloud account.

### Devices

When setup the component will scan for AwoX BLE mesh devices and publish [discovery](https://www.home-assistant.io/integrations/mqtt/#mqtt-discovery) messages for each device on MQTT. When using HomeAssistant the device will show up under the MQTT integration. And you can (re)name the devices there.

Each device type/product can be configured by `product_id` so it gets the correct features in HomeAssistant.

The `product id` needed for the configuration is shown in model string of the mqtt discovery message/HomeAssistant device info. [See yaml options](#device_info)

<details>
  <summary>Config examples known devices</summary>

```yaml
device_info:
-  product_id: 0x13
   device_type: RGB
   name: SmartLIGHT Color Mesh 9
   model: SMLm_C9
   manufacturer: AwoX

-  product_id: 0x14
   device_type: WHITE_TEMP
   name: SmartLIGHT White Mesh 13W
   model: SMLm_W13
   manufacturer: AwoX

-  product_id: 0x15
   device_type: RGB
   name: SmartLIGHT Color Mesh 13W
   model: SMLm_C13
   manufacturer: AwoX

-  product_id: 0x16
   device_type: WHITE_TEMP
   name: SmartLIGHT White Mesh 15W
   model: SMLm_W15
   manufacturer: AwoX

-  product_id: 0x17
   device_type: RGB
   name: SmartLIGHT Color Mesh 15W
   model: SMLm_C15
   manufacturer: AwoX

-  product_id: 0x21
   device_type: WHITE_TEMP
   name: SmartLIGHT White Mesh 9W
   model: SSMLm_w9
   manufacturer: AwoX

-  product_id: 0x22
   device_type: RGB
   name: SmartLIGHT Color Mesh 9W
   model: SSMLm_c9
   manufacturer: AwoX

-  product_id: 0x23
   device_type: RGB
   name: EGLOBulb A60 9W
   model: ESMLm_c9
   manufacturer: EGLO

-  product_id: 0x24
   device_type: RGB
   name: Keria SmartLIGHT Color Mesh 9W
   model: KSMLm_c9
   manufacturer: KERIA

-  product_id: 0x25
   device_type: RGB
   name: EGLOPanel 30X30
   model: EPanel_300
   manufacturer: EGLO

-  product_id: 0x26
   device_type: RGB
   name: EGLOPanel 60X60
   model: EPanel_600
   manufacturer: EGLO

-  product_id: 0x27
   device_type: RGB
   name: EGLO Ceiling DOWNLIGHT
   model: EMod_Ceil
   manufacturer: EGLO

-  product_id: 0x29
   device_type: RGB
   name: EGLOBulb G95 13W
   model: ESMLm_c13g
   manufacturer: EGLO

-  product_id: 0x2A
   device_type: RGB
   name: Keria SmartLIGHT Color Mesh 13W Globe
   model: KSMLm_c13g
   manufacturer: KERIA

-  product_id: 0x2B
   device_type: RGB
   name: SmartLIGHT Color Mesh 13W Globe
   model: SMLm_c13g
   manufacturer: AwoX

-  product_id: 0x30
   device_type: RGB
   name: EGLOPanel 30X120
   model: EPanel_120
   manufacturer: EGLO

-  product_id: 0x32
   device_type: RGB
   name: Spot 120
   model: EGLOSpot 120/w
   manufacturer: EGLO
   icon: mdis:wall-sconce-flat

-  product_id: 0x33
   device_type: RGB
   name: Spot 170
   model: EGLOSpot 170/w
   manufacturer: EGLO
   icon: mdi:wall-sconce-flat

-  product_id: 0x34
   device_type: RGB
   name: Spot 225
   model: EGLOSpot 225/w
   manufacturer: EGLO
   icon: mdi:wall-sconce-flat

-  product_id: 0x35
   device_type: RGB
   name: Giron-C 17W
   model: EGLO 32589
   manufacturer: EGLO
   icon: mdi:wall-sconce-flat

-  product_id: 0x36
   device_type: RGB
   name: EGLO Ceiling GIRON 30
   model: ECeil_g38
   manufacturer: EGLO

-  product_id: 0x37
   device_type: RGB
   name: SmartLIGHT Color Mesh 5W GU10
   model: SMLm_c5_GU10
   manufacturer: AwoX

-  product_id: 0x38
   device_type: RGB
   name: SmartLIGHT Color Mesh 5W E14
   model: SMLm_c5_E14
   manufacturer: AwoX

-  product_id: 0x3A
   device_type: RGB
   name: Keria SmartLIGHT Color Mesh 5W GU10
   model: KSMLm_c5_GU10
   manufacturer: KERIA

-  product_id: 0x3B
   device_type: RGB
   name: Keria SmartLIGHT Color Mesh 5W E14
   model: KSMLm_c5_E14
   manufacturer: KERIA

-  product_id: 0x3C
   device_type: RGB
   name: SmartLIGHT Color Mesh 5W GU10
   model: ESMLm_c5_GU10
   manufacturer: EGLO

-  product_id: 0x3D
   device_type: RGB
   name: SmartLIGHT Color Mesh 5W E14
   model: ESMLm_c5_E14
   manufacturer: EGLO

-  product_id: 0x3F
   device_type: RGB
   name: EGLO Surface round
   model: EFueva_225r
   manufacturer: EGLO

-  product_id: 0x40
   device_type: RGB
   name: EGLO Surface square
   model: EFueva_225s
   manufacturer: EGLO

-  product_id: 0x41
   device_type: RGB
   name: EGLO Surface round
   model: EFueva_300r
   manufacturer: EGLO

-  product_id: 0x42
   device_type: RGB
   name: EGLO Surface square
   model: EFueva_300s
   manufacturer: EGLO

-  product_id: 0x43
   device_type: RGB
   name: SmartLIGHT Color Mesh 9W
   model: SMLm_c9s
   manufacturer: AwoX

-  product_id: 0x44
   device_type: RGB
   name: SmartLIGHT Color Mesh 13W
   model: SMLm_c13gs
   manufacturer: AwoX

-  product_id: 0x45
   device_type: RGB
   name: EGLOBulb A60 9W
   model: ESMLm_c9s
   manufacturer: EGLO

-  product_id: 0x46
   device_type: RGB
   name: EGLOBulb G95 13W
   model: ESMLm_c13gs
   manufacturer: EGLO

-  product_id: 0x47
   device_type: RGB
   name: Keria SmartLIGHT Color Mesh 9W
   model: KSMLm_c9s
   manufacturer: KERIA

-  product_id: 0x48
   device_type: RGB
   name: Keria SmartLIGHT Color Mesh 13W Glob
   model: KSMLm_c13gs
   manufacturer: KERIA

-  product_id: 0x49
   device_type: DIM
   name: EGLOBulb A60 Warm
   model: ESMLm_w9w
   manufacturer: EGLO

-  product_id: 0x4A
   device_type: DIM
   name: EGLOBulb A60 Neutral
   model: ESMLm_w9n
   manufacturer: EGLO

-  product_id: 0x4B
   device_type: RGB
   name: EGLO Ceiling
   model: ECeiling_30
   manufacturer: EGLO

-  product_id: 0x4C
   device_type: RGB
   name: EGLO Pendant
   model: EPendant_30
   manufacturer: EGLO

-  product_id: 0x4D
   device_type: RGB
   name: EGLO Pendant
   model: EPendant_20
   manufacturer: EGLO

-  product_id: 0x4E
   device_type: RGB
   name: EGLO Stripled 3m
   model: EStrip_3m
   manufacturer: EGLO

-  product_id: 0x4F
   device_type: RGB
   name: EGLO Stripled 5m
   model: EStrip_5m
   manufacturer: EGLO

-  product_id: 0x50
   device_type: DIM
   name: Outdoor
   model: EOutdoor_w14w
   manufacturer: EGLO

-  product_id: 0x51
   device_type: RGB
   name: EGLOSpot
   model: ETriSpot_85
   manufacturer: EGLO

-  product_id: 0x53
   device_type: RGB
   name: SmartLIGHT Color Mesh 9W
   model: SMLm_c9i
   manufacturer: AwoX

-  product_id: 0x54
   device_type: RGB
   name: EGLOBulb A60 9W
   model: ESMLm_c9i
   manufacturer: EGLO

-  product_id: 0x55
   device_type: RGB
   name: Keria SmartLIGHT Color Mesh 9W
   model: KSMLm_c9i
   manufacturer: KERIA

-  product_id: 0x56
   device_type: RGB
   name: EGLOPanel 62X62
   model: EPanel_620
   manufacturer: EGLO

-  product_id: 0x57
   device_type: RGB
   name: EGLOPanel 45X45
   model: EPanel_450
   manufacturer: EGLO

-  product_id: 0x59
   device_type: RGB
   name: SmartLIGHT Color Mesh 13W Globe
   model: SMLm_c13gi
   manufacturer: AwoX

-  product_id: 0x5A
   device_type: RGB
   name: EGLOBulb G95 13W
   model: ESMLm_c13gi
   manufacturer: EGLO

-  product_id: 0x5B
   device_type: RGB
   name: Keria SmartLIGHT Color Mesh 13W Globe
   model: KSMLm_c13gi
   manufacturer: KERIA

-  product_id: 0x5C
   device_type: RGB
   name: SmartLIGHT Color Mesh 9W
   model: SSMLm_c9i
   manufacturer: AwoX

-  product_id: 0x64
   device_type: WHITE_TEMP
   name: SmartLIGHT White Mesh 9W
   model: SMLm_w9
   manufacturer: AwoX

-  product_id: 0x65
   device_type: WHITE_TEMP
   name: SmartLIGHT White Mesh 9W
   model: ESMLm_w9
   manufacturer: EGLO

-  product_id: 0x69
   device_type: RGB
   name: Ceiling GIRON 60
   model: ECeil_g60
   manufacturer: EGLO

-  product_id: 0x6A
   device_type: WHITE_TEMP
   name: SmartLIGHT Bulb A60 Warm
   model: SMLm_w9w
   manufacturer: AwoX

-  product_id: 0x6F
   device_type: WHITE_TEMP
   name: EGLOBulb Filament G80
   model: ESMLFm-w6-G80
   manufacturer: EGLO

-  product_id: 0x71
   device_type: WHITE_TEMP
   name: EGLOBulb Filament ST64
   model: ESMLFm-w6-ST64
   manufacturer: EGLO

-  product_id: 0x75
   device_type: WHITE_TEMP
   name: EGLOBulb Filament G95
   model: ESMLFm-w6-G95
   manufacturer: EGLO

-  product_id: 0x77
   device_type: RGB
   name: EGLO Spot
   model: ESpot_c5
   manufacturer: EGLO

-  product_id: 0x78
   device_type: RGB
   name: EGLO Fraioli
   model: EFraioli_c17
   manufacturer: EGLO

-  product_id: 0x79
   device_type: RGB
   name: EGLO Frattina
   model: EFrattina_c18
   manufacturer: EGLO

-  product_id: 0x7A
   device_type: RGB
   name: EGLO Frattina
   model: EFrattina_c27
   manufacturer: EGLO

-  product_id: 0x7B
   device_type: RGB
   name: EGLOPanel 30 Round
   model: EPanel_r300
   manufacturer: EGLO

-  product_id: 0x7C
   device_type: RGB
   name: EGLOPanel 45 Round
   model: EPanel_r450
   manufacturer: EGLO

-  product_id: 0x7D
   device_type: RGB
   name: EGLOPanel 60 Round
   model: EPanel_r600
   manufacturer: EGLO

-  product_id: 0x7E
   device_type: RGB
   name: EGLOPanel 10X120
   model: EPanel_120_10
   manufacturer: EGLO

-  product_id: 0x80
   device_type: WHITE_TEMP
   name: EPanel white round
   model: EPanel_w_round
   manufacturer: EGLO

-  product_id: 0x81
   device_type: WHITE_TEMP
   name: EPanel white square
   model: EPanel_w_square
   manufacturer: EGLO

-  product_id: 0x82
   device_type: WHITE_TEMP
   name: EPanel white rectangle
   model: EPanel_w_rect
   manufacturer: EGLO

-  product_id: 0x83
   device_type: WHITE_TEMP
   name: ECeiling white round
   model: ECeiling-w
   manufacturer: EGLO

-  product_id: 0x87
   device_type: WHITE_TEMP
   name: EGLO Tunable White
   model: EDoubleWhite
   manufacturer: EGLO

-  product_id: 0x88
   device_type: RGB
   name: EGLO Ceiling GIRON 80
   model: ECeil_g80
   manufacturer: EGLO

-  product_id: 0x89
   device_type: WHITE_TEMP
   name: Outdoor Marchesa-C
   model: EMarchesa_C
   manufacturer: EGLO

-  product_id: 0x8A
   device_type: WHITE_TEMP
   name: Outdoor Francari-C
   model: EFrancari_C
   manufacturer: EGLO

-  product_id: 0x92
   device_type: RGB
   name: EPanel square
   model: EPanel_36W_square
   manufacturer: EGLO

-  product_id: 0x94
   device_type: DIM
   name: EGLOBulb Filament ST64
   model: ESMLFm-w6w-ST64
   manufacturer: EGLO

-  product_id: 0x95
   device_type: DIM
   name: EGLOBulb Filament G95
   model: ESMLFm-w6w-G95
   manufacturer: EGLO

-  product_id: 0x96
   device_type: RGB
   name: EGLO RGB+TW
   model: EGLO-RGB-TW
   manufacturer: EGLO

-  product_id: 0x97
   device_type: WHITE_TEMP
   name: EGLO Tunable White
   model: EGLO-TW
   manufacturer: EGLO

-  product_id: 0x99
   device_type: RGB
   name: EGLO RGB+TW
   model: EGLO-RGB-TW
   manufacturer: EGLO

-  product_id: 0x9A
   device_type: WHITE_TEMP
   name: EGLO Tunable White
   model: JBT_Gen_CCT_1
   manufacturer: EGLO

-  product_id: 0x9B
   device_type: DIM
   name: EGLO Tunable White
   model: JBT_Gen_Dim_1
   manufacturer: EGLO

-  product_id: 0xA1
   device_type: RGB
   name: EGLOLed Relax
   model: ELedRelax
   manufacturer: EGLO

-  product_id: 0xA2
   device_type: RGB
   name: EGLOLed Stripe
   model: ELedStripe
   manufacturer: EGLO

-  product_id: 0xA3
   device_type: RGB
   name: EGLOLed Plus
   model: ELedPlus
   manufacturer: EGLO

-  product_id: 0xA4
   device_type: WHITE_TEMP
   name: EGLOLed Plus TW
   model: ELedPlus-TW
   manufacturer: EGLO

-  product_id: 0xA5
   device_type: DIM
   name: EGLOLed Plus Dimmable
   model: ELedPlus-Dimm
   manufacturer: EGLO

-  product_id: 0xA6
   device_type: WHITE_TEMP
   name: EGLOBulb
   model: ESMLFm-w6-TW
   manufacturer: EGLO

-  product_id: 0xA7
   device_type: DIM
   name: EGLOBulb
   model: ESMLFm-w6-Dimm
   manufacturer: EGLO

-  product_id: 0xA8
   device_type: RGB
   name: ECeiling square
   model: ECeiling-24W-square
   manufacturer: EGLO

-  product_id: 0xA9
   device_type: RGB
   name: EGLO RGB+TW
   model: EGLO-RGB-TW-IPSU
   manufacturer: EGLO

-  product_id: 0xAA
   device_type: WHITE_TEMP
   name: EGLO Tunable White
   model: EGLO-TW-IPSU
   manufacturer: EGLO

-  product_id: 0xAC
   device_type: RGB
   name: EGLO frameless
   model: EPanel-Frameless
   manufacturer: EGLO

-  product_id: 0xAD
   device_type: WHITE_TEMP
   name: EGLO Tunable White
   model: EDoubleWhite-ipsu
   manufacturer: EGLO

-  product_id: 0x97
   device_type: PLUG
   name: EGLO PLUG PLUS
   model: SMPWBm10AUS
   manufacturer: EGLO
   icon: mdi:power-socket-au

-  product_id: 0x9E
   device_type: PLUG
   name: EGLO PLUG PLUS
   model: SMPWBm10AUSb
   manufacturer: EGLO
   icon: mdi:power-socket-au

-  product_id: 0x90
   device_type: PLUG
   name: EGLO PLUG PLUS
   model: SMPWBm10CH
   manufacturer: EGLO
   icon: mdi:power-socket-ch

-  product_id: 0xA0
   device_type: PLUG
   name: EGLO PLUG PLUS
   model: SMPWBm10CHb
   manufacturer: EGLO
   icon: mdi:power-socket-ch

-  product_id: 0x67
   device_type: PLUG
   name: EGLO PLUG PLUS
   model: SMPWBm10FR
   manufacturer: EGLO
   icon: mdi:power-socket-fr

-  product_id: 0x84
   device_type: PLUG
   name: EGLO PLUG PLUS
   model: SMPWBm10FRa
   manufacturer: EGLO
   icon: mdi:power-socket-fr

-  product_id: 0x9C
   device_type: PLUG
   name: EGLO PLUG PLUS
   model: SMPWBm10FRb
   manufacturer: EGLO
   icon: mdi:power-socket-fr

-  product_id: 0x68
   device_type: PLUG
   name: EGLO PLUG PLUS
   model: SMPWBm10GE
   manufacturer: EGLO
   icon: mdi:power-socket-de

-  product_id: 0x85
   device_type: PLUG
   name: EGLO PLUG PLUS
   model: SMPWBm10GEa
   manufacturer: EGLO
   icon: mdi:power-socket-de

-  product_id: 0x9D
   device_type: PLUG
   name: EGLO PLUG PLUS
   model: SMPWBm10GEb
   manufacturer: EGLO
   icon: mdi:power-socket-de

-  product_id: 0x8F
   device_type: PLUG
   name:  EGLO PLUG PLUS
   model:  SMPWBm10UK
   manufacturer: EGLO
   icon: mdi:power-socket-uk

-  product_id: 0x9F
   device_type: PLUG
   name: EGLO PLUG PLUS
   model: SMPWBm10UKb
   manufacturer: EGLO
   icon: mdi:power-socket-uk

-  product_id: 0x8B
   device_type: PLUG
   name: EGLO PLUG
   model: ESMP-Bm10-AUS
   manufacturer: EGLO
   icon: mdi:power-socket-au

-  product_id: 0x8D
   device_type: PLUG
   name: EGLO PLUG
   model: ESMP-Bm10-CH
   manufacturer: EGLO
   icon: mdi:power-socket-ch

-  product_id: 0x62
   device_type: PLUG
   name: EGLO PLUG
   model: ESMP-Bm10-FR
   manufacturer: EGLO
   icon: mdi:power-socket-fr

-  product_id: 0x63
   device_type: PLUG
   name: EGLO PLUG
   model: ESMP-Bm10-GE
   manufacturer: EGLO
   icon: mdi:power-socket-de

-  product_id: 0x8C
   device_type: PLUG
   name: EGLO PLUG
   model: ESMP-Bm10-UK
   manufacturer: EGLO
   icon: mdi:power-socket-uk

```
</details>

### Groups/zones

The groups (called zone in AwoxHomeControl) are automaticly discovered as lights with the group id as name (example: `Group 3`).

When using HomeAssistant the groups will show up as a device under the MQTT integration. And you can (re)name the group/device there.

When 1 light in a group is `on` the group will be shown as `on`. If 1 light in the group is `online` the group will be shown `online`.

### YAML options

For an example yaml see [`awox-ble-mesh-hub.yaml`](awox-ble-mesh-hub.yaml). For a full getting started see [#71](https://github.com/fsaris/EspHome-AwoX-BLE-mesh-hub/discussions/71).

The available `awox_mesh:` config options:

#### `mesh_name` _(string)_

To connect to you device you need the `mesh_name` and `mesh_password` easiest way to find/get these is to use [this form](https://fsaris.github.io/EspHome-AwoX-BLE-mesh-hub/awoxh-mesh-credentials-tool/) to read them from your AwoX Cloud account.


#### `mesh_password` _(string)_

See [`mesh_name`](#mesh_name-string)


#### `address_prefix` _(string - OPTIONAL)_

The device will scan for available BLE devices and will use this prefix to filter discovered devices by MAC address.

###### _Default value: `A4:C1`_


#### `min_rssi` _(number - OPTIONAL)_

The device will scan for available BLE devices and will use this value to filter discovered devices by the RSSI value.

###### _Default value: -90_


#### `allowed_mac_addresses` _(list of MAC addresses - OPTIONAL)_

Instead of the `address_prefix` option you can configure a list of mac addresses that the hub is allowed to connect to. This enables you to exclude some devices that are often offline or to create multiple "mesh" networks when combined with the `allowed_mesh_ids` option.

###### _Example:_
```yaml
  allowed_mac_addresses:
    - A4:C1:38:8A:BA:11
    - A4:C1:38:65:AD:22
```


#### `allowed_mesh_ids` _(list of numbers - OPTIONAL)_

You can give a list of mesh_ids that only should be handled by this hub (ESPHome device).
For example when you want to setup multiple ESP modules to only control a part of the mesh. Be sure to also set `allowed_mac_addresses` option to prevent that 1 device tries to connect to a mesh device that's not part of your `allowed_mesh_ids` list.

###### _Example:_
```yaml
 allowed_mesh_ids:
   - 12345
   - 23456
   - 6758
```


#### `max_connections` _(number, min: 1, max: 3 - OPTIONAL)_

In some situations the device that the hub connects to isn't able to reach the next device in the mesh. Then it can help that the hub connects to a different/additional device to be able to reach all device.

###### _Default value: 2_


#### `device_info`

List of device type descriptions.

###### _Example:_
```yaml
  device_info:
    - product_id: 0x32
      device_type: RGB
      manufacturer: EGLO
      name: Spot 120
      model: EGLOSpot 120/w
      icon: mdi:wall-sconce-flat
```

For each `product_id` the available features and name + icon can be configured. Unknown devices will be recognized as a dimmable device (`DIM`).

##### `product_id` _(hex value)_

Each product type has a unique `product_id` ([see above](#devices) how to find the `product_id` of your devices).

##### `device_type` _(RGB, DIM, WHITE_TEMP, PLUG)_

- `RGB` - RGB device
- `DIM` - dimmable device
- `WHITE_TEMP` - Dimmable and adjustable white color device
- `PLUG` - Plug (or on/off) device

##### `manufacturer` _(string)_

The name of the product manufacturer.

######  _Example value `EGLO`_


##### `name`

The product name.

###### _Example value `Spot 120`_


##### `model`

The product model name.

###### _Example value `EGLOSpot 120/w`_

##### `icon` _(string representing a mdi icon - OPTIONAL)_

The product icon used for example in HomeAssistent as device icon.

###### _Example value: `mdi:wall-sconce-flat`_


### General YAML / Setup remarks

- Disable Home Assistant API (`api:`) to prevent frequent MQTT disconnections
- Use the `esp-idf` as framework type

#### Hidden SSID WLAN
In case of using a WLAN with hidden SSID, mind to use the multiple network option, to define the hidden variable of the wifi network: [Connecting to Multiple Networks](https://esphome.io/components/wifi.html#connecting-to-multiple-networks)


#### Ethernet instead of Wi-Fi (ESP32-S3, Seeed XIAO W5500 Ethernet Adapter)

The component does not use Wi-Fi itself, only MQTT, so the hub can run on a wired connection. A ready-to-use example for the
[Seeed Studio XIAO W5500 Ethernet Adapter](https://wiki.seeedstudio.com/xiao_w5500_ethernet_adapter/) (XIAO ESP32-S3 Plus + WIZnet W5500, powered by PoE or USB-C)
is in [awox-ble-mesh-hub-xiao-esp32s3-w5500.yaml](awox-ble-mesh-hub-xiao-esp32s3-w5500.yaml).

Differences from the default [awox-ble-mesh-hub.yaml](awox-ble-mesh-hub.yaml):

- `board: seeed_xiao_esp32s3` and `variant: esp32s3` (see the [PlatformIO board](https://docs.platformio.org/en/latest/boards/espressif32/seeed_xiao_esp32s3.html))
- the `wifi:` section is replaced by an `ethernet:` section of type `W5500` with the pins from the
  [Seeed ESPHome guide](https://wiki.seeedstudio.com/esphome_xiao_w5500_ethernet_adapter/):

  ```yaml
  ethernet:
    type: W5500
    clk_pin: GPIO7
    mosi_pin: GPIO9
    miso_pin: GPIO8
    cs_pin: GPIO2
    interrupt_pin: GPIO10
  ```

Remarks:

- Seeed documents the ESPHome setup only for boards produced after November 1st 2025 (hardware revision V1.2, which fixed the `INT` pin wiring). Check the pins against the schematic if the Ethernet link does not come up with an older board.
- The first flash has to be done over USB-C. Afterwards `esphome upload --device <ip-address>` works over Ethernet. The `.local` mDNS name is not resolved across VLANs/subnets, use the IP address.
- The W5500 sends untagged frames. If you use VLANs, plug the board into an access port of the VLAN where your MQTT broker is reachable.
- Without PoE the board needs 5 V on USB-C; a plain Ethernet cable only carries data.
- Without a cable the log shows `Connecting failed; reconnecting` every 15 seconds. This is normal and does not mean the pins are wrong.
- Other boards with a W5500 use the same [`ethernet:`](https://esphome.io/components/ethernet/) component, with their own board, variant and pins.

### Requirements
- ESP32 module (Wi-Fi or Ethernet; the ESP32-S3 with a W5500 is described [above](#ethernet-instead-of-wi-fi-esp32-s3-seeed-xiao-w5500-ethernet-adapter))
- ESPHome 2025.12.0 or newer
- MQTT broker

### Recomendations
- HomeAssistant 2023.8.0 or newer
- ESPHome add-on for HomeAssistant


### References
- https://github.com/fsaris/home-assistant-awox
- https://github.com/Leiaz/python-awox-mesh-light
- https://github.com/vpaeder/telinkpp


## Fork development and regression checks

This fork includes [GitHub Actions](.github/workflows/regression.yml) that
runs Python source regression tests, native C++ protocol-boundary assertions,
and an ESPHome 2026.10.2 ESP-IDF firmware build using the **checked-out local
AwoX component source**. GitHub CI does not test live BLE connections or
Home Assistant state transitions; test those on an actual ESP32 before
deploying changes to a production gateway.

Run the fast checks locally:

```bash
python3 -m unittest discover -s tests -p 'test_*.py' -v
c++ -std=c++17 -Wall -Wextra -Werror -pedantic -I. tests/test_protocol_safety.cpp -o /tmp/awox_protocol_tests
/tmp/awox_protocol_tests
```

To test this fork in ESPHome, replace the upstream Git source with
`https://github.com/realholgi/EspHome-AwoX-BLE-mesh-hub` and reference
`main` or a specific tested commit. Keep `max_connections: 1` on
single-device deployments until the multi-slot fix is validated on hardware.

## Home Assistant BLE diagnostics (fork)

The fork publishes per-connection diagnostics through MQTT discovery. All
diagnostic entities share the **existing** `<mqtt_topic_prefix>/connection_status`
JSON payload; they do not open new BLE connections or subscribe to new mesh
devices.

The following entities are **enabled by default**:

| Sensor | Meaning |
| --- | --- |
| AwoX active BLE connections | Number of established BLE client slots |
| AwoX online mesh devices | Unique mesh IDs reachable through active slots |
| Connection 0 RSSI | Last observed BLE advertisement RSSI in dBm |
| Connection 0 attempts | Connection attempts since the last ESP reboot |
| Connection 0 disconnects | Completed GATT sessions that ended |
| Connection 0 last disconnect reason | Last observed GATT reason, e.g. `0x08` |
| Connection 0 last notification age | Seconds since the last valid-length AwoX notification |

Additional diagnostic entities are available but **disabled by default**:
failed connection attempts, pending command count, failed command writes and
dropped commands. Enable them from the MQTT device's entity list in Home
Assistant as needed.

The diagnostic sensor names contain `Connection 0` for the first slot,
`Connection 1` for the second, and so on. Counts are reset by ESP reboot.
The elapsed notification time is unavailable until the first valid-length
notification arrives. RSSI is the last observed advertisement signal; it is
**not** a continuous measurement of active-link signal strength. The last
disconnect reason may be unavailable: low-level HCI failures such as `0x3E`
do not always produce a GATT disconnect event, and the firmware does not
fabricate a reason in that case.

To limit MQTT and ESP32 load, the `connection_status` message is updated
immediately on relevant connection-status changes and periodically about
every **30 seconds** while connected. The data is therefore not a
high-frequency BLE telemetry feed. The MQTT diagnostic entity IDs include
the gateway MAC and connection slot to avoid collisions across gateways.

**For current users:** No new `awox_mesh:` YAML parameters are required.
Build the firmware from this fork instead of the upstream to get the new
entities. The new sensors appear after MQTT discovery runs, provided MQTT
discovery is enabled. If you use `discovery_retain: false`, ensure Home
Assistant is online during initial discovery or restart the gateway after
Home Assistant reconnects.

## M5StickC Plus local status display

The [three-page M5StickC Plus example](sample-config/m5stickcplus-gateway-display.yaml)
uses ESPHome 2026.10.2's `mipi_spi` driver with the known working
M5StickC Plus SPI pins: CLK GPIO13, MOSI GPIO15, CS GPIO5,
DC GPIO23, reset GPIO18 and 270-degree rotation. It uses the `T-DISPLAY`
135x240 panel geometry/offsets; the pin overrides are essential.

- **Page 1 (overview)**: AwoX gateway connection, EGLO panel status,
  RSSI (last advertisement), online mesh devices, last notification age
  and battery percentage.
- **Page 2 (diagnostics)**: connection attempts, disconnects, failed
  attempts, GATT disconnect reason, pending commands, write failures,
  dropped commands, RSSI and free heap.
- **Page 3 (system)**: Wi-Fi RSSI, device uptime, cached EGLO panel firmware revision and BLE link status. Values are local display data and are not exposed as additional Home Assistant entities.
- **Red LED (GPIO10)**: active-low, normally off. After at least 60 seconds without a BLE link, blink for 150ms every 5 seconds; automatically stop after reconnection. No additional BLE requests.
- **Button A (GPIO37)**: wake the display via the AXP192 switch.
- **Button B (GPIO39)**: cycle through the three pages and wake the display.
- **Power behavior**: AXP192 retains the display-brightness control and
  its existing `auto_off: 5min`. Only the backlight rail is switched;
  Wi-Fi, BLE mesh control and MQTT continue to run.

The display takes a read-only snapshot directly from `awox_mesh` with no
new BLE requests or MQTT loopback. A partial 8-bit framebuffer (25%) keeps
the memory requirement lower than a full 16-bit framebuffer, and the
display redraw interval is three seconds. The example is intended for
one AwoX mesh lamp (`max_connections: 1`). Check ESPHome compilation
before flashing, and verify heap and BLE uptime on physical hardware.

The example includes the AXP192 component from
`realholgi/esphome-axp192`; if you already have a working YAML, copy
only its `spi`, `font`, `globals`, `display` and GPIO39 button sections,
and assign `id: awox_gateway` to your existing `awox_mesh` component.
Do not duplicate existing AXP192 sensors, GPIO37 button or `i2c` setup.


## Optional firmware and hardware revisions on AwoX devices

The gateway attempts to read the standard BLE Device Information Service
(`0x180A`) firmware revision (`0x2A26`) and hardware revision (`0x2A27`)
**after successful AwoX mesh authentication**. Each characteristic is read
at most once per successful BLE session. Neither operation triggers a second
BLE connection or requires extra `awox_mesh:` YAML fields.

If the lamp implements either standard GATT characteristic and returns a
printable, valid revision string, the result is cached under the lamp's BLE
MAC address and published as `device.sw_version` and/or `device.hw_version`
in its existing Home Assistant MQTT discovery entity. The existing discovery
identity and device association remain unchanged. Once a revision arrives,
the gateway republishes the device's discovery configuration; a late mesh
identity resolution is also supported.

Some AwoX/EGLO devices might not expose these GATT characteristics. In that
case the version fields are omitted rather than guessed; normal mesh control
continues. The feature **reads revision strings only**, not a firmware image,
and does not perform an OTA update. Check the ESPHome logs for
`Read firmware revision:` or `Read hardware revision:` and view the
EGLO panel's *Device info* in Home Assistant.

The version cache is in RAM and is repopulated on the next successful BLE
session after a gateway reboot. The gateway's own ESPHome build version is
not used as the panel's firmware version.
