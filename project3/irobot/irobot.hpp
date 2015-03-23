#ifndef IROBOT_COMM_INCLUDED
#define IROBOT_COMM_INCLUDED


namespace irobot {

enum opcode
{
    // start/stop
    op_start = 128,
    op_reset = 7,
    op_stop = 173,
    op_power_down = 133,

    // baud rate
    op_baud = 129,
    // communication mode
    op_safe_mode = 131,
    op_full_mode = 132,
    // automatic operation modes
    op_clean_default = 135,
    op_clean_max = 136,
    op_clean_spot = 134,
    op_seek_dock = 143,
    // config
    op_schedule = 167,
    op_set_day_time = 168,
    // actuators
    op_drive = 137,
    op_drive_direct = 145,
    op_drive_pwm = 146,
    op_motors = 138,
    op_motors_pwm = 144,
    op_leds = 139,
    op_leds_schedule = 162,
    op_leds_digits_raw = 163,
    op_leds_digits_ascii = 164,
    op_buttons = 165,
    op_song_compose = 140,
    op_song_play = 141,

    // sensors
    op_sensor = 142,
    op_sensor_list = 149,
    op_sensor_stream = 148,
    op_sensor_stream_active = 150
};

enum sensor
{
    sense_bumps_and_wheel_drops = 7,
    sense_wall = 8,
    sense_cliff_left = 9,
    sense_cliff_front_left = 10,
    sense_cliff_front_right = 11,
    sense_cliff_rigth = 12,
    sense_virtual_wall = 13,
    sense_wheel_overcurrent = 14,
    sense_dirt_detect = 15,
    sense_unused_byte = 16,
    sense_infrared_omni = 17,
    sense_infrared_left = 52,
    sense_infrared_right = 53,
    sense_buttons = 18,
    sense_distance = 19,
    sense_angle = 20,
    sense_charging = 21,
    sense_voltage = 22,
    sense_current = 23,
    sense_temperature = 24,
    sense_battery_charge = 25,
    sense_battery_capacity = 26,
    sense_wall_signal = 27,
    sense_cliff_left_signal = 28,
    sense_cliff_front_left_signal = 29,
    sense_cliff_front_right_signal = 30,
    sense_cliff_right_signal = 31,
    sense_charge_sources = 34,
    sense_oi_mode = 35,
    sense_song_number = 36,
    sense_song_playing = 37,
    sense_num_stream_packets = 38,
    sense_velocity_request = 39,
    sense_radius_request = 40,
    sense_right_velocity_request = 41,
    sense_left_velocity_request = 42,
    sense_left_encoder_count = 43,
    sense_right_encoder_count = 44,
    sense_light_bumper = 45,
    sense_light_bump_left_signal = 46,
    sense_light_bump_front_left_signal = 47,
    sense_light_bump_center_left_signal = 48,
    sense_light_bump_center_right_signal = 49,
    sense_light_bump_front_right_signal = 50,
    sense_light_bump_right_signal = 51,
    sense_motor_left_current = 54,
    sense_motor_right_current = 55,
    sense_brush_main_current = 56,
    sense_brush_side_current = 57,
    sense_stasis = 58
};

}

#endif // IROBOT_COMM_INCLUDED
