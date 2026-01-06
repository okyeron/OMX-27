Import("env")

env.AddCustomTarget(
    name="clear-storage",
    dependencies=None,
    actions=[
        "echo 'Clearing FRAM/EEPROM storage'",
        "teensy_reboot -s",
        "teensy_loader_cli -mmcu=mk20dx256 -w -s -v clear_storage/clear_storage.ino.hex",
        "echo \"\\nScript uploaded: When complete, reflash the device with 'pio run -t upload'\\n\"",
    ],
    title="Clear Storage",
    description="Clear FRAM / EEPROM storage"
)
