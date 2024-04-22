autoload, "tao-rt.i",
    tao_abort,
    tao_attach_remote_camera,
    tao_attach_remote_mirror,
    tao_attach_shared_array,
    tao_attach_shared_object,
    tao_capture,
    tao_config_read_shmid,
    tao_configure,
    tao_create_remote_object,
    tao_create_rwlocked_object,
    tao_create_shared_array,
    tao_create_shared_object,
    tao_debug,
    tao_ensure_unlocked,
    tao_get_current_time,
    tao_get_data,
    tao_get_image_shmid,
    tao_get_members,
    tao_get_monotonic_time,
    tao_get_monotonic_time_origin,
    tao_get_serial,
    tao_get_state_name,
    tao_get_timestamp,
    tao_is_locked,
    tao_is_shared_object,
    tao_kill,
    tao_lock,
    tao_rdlock,
    tao_reset,
    tao_save_fits,
    tao_send_commands,
    tao_set_data,
    tao_set_reference,
    tao_set_perturbation,
    tao_set_serial,
    tao_set_timestamp,
    tao_sleep,
    tao_start,
    tao_stop,
    tao_type,
    tao_unlock,
    tao_wait_command,
    tao_wait_frame,
    tao_wait_image,
    tao_wait_output,
    tao_wrlock;

autoload, "tao-rt-tests.i",
    tao_analyze_latency,
    tao_measure_latency,
    tao_print_tree;
