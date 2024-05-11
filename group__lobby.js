var group__lobby =
[
    [ "pkrsrv_lobby_t", "group__lobby.html#structpkrsrv__lobby", [
      [ "ref_counted", "group__lobby.html#afca80da4936e4b2a28c541b2c465554d", null ],
      [ "server", "group__lobby.html#a5b69199476a375ffcae37a63ad06ce51", null ],
      [ "mutex", "group__lobby.html#a2b31d75198e68f8b682a6c4d90ac7adf", null ],
      [ "process_latency", "group__lobby.html#ad29bc53c0fc4a1cdf382b7f18a30f275", null ],
      [ "sessions", "group__lobby.html#a486ba70ab97f21e855381fe81a81647c", null ],
      [ "eventloop", "group__lobby.html#a8315bc530a0227ea47a91e5acdcdffa3", null ]
    ] ],
    [ "pkrsrv_lobby_sessions_t", "group__lobby.html#structpkrsrv__lobby__sessions", [
      [ "pkrsrv_lobby_sessions_add", "group__lobby.html#gad97ec74688302a19a98490613e153c85", null ],
      [ "pkrsrv_lobby_sessions_remove", "group__lobby.html#ga589399d85a561cb14067720c6b276522", null ],
      [ "next", "group__lobby.html#a1c70cfae0b3b6c9a4158f90b90ce640b", null ],
      [ "prev", "group__lobby.html#a6ab6edb4392a603307989aedeac1de43", null ],
      [ "terminal", "group__lobby.html#ab7c02d7c5ff849c5c297dec3be0975ab", null ],
      [ "length", "group__lobby.html#ad0382fec64c02d2f14c5fca074493789", null ]
    ] ],
    [ "pkrsrv_lobby_session_t", "group__lobby.html#structpkrsrv__lobby__session", [
      [ "pkrsrv_lobby_session_new", "group__lobby.html#ga8b101a21efde6525bb426d01737079c0", null ],
      [ "pkrsrv_lobby_session_free", "group__lobby.html#ga5f5427dfbd357951c90f1a5a32a238e3", null ],
      [ "pkrsrv_lobby_session_start", "group__lobby.html#ga121deaeff46406fcc61699498efeb499", null ],
      [ "pkrsrv_lobby_session_stop", "group__lobby.html#gaf74ed9186c5506ca9b7362828d9250ac", null ],
      [ "pkrsrv_lobby_session_getby_id", "group__lobby.html#gaf54d795b883b8c35a3bbe5a004329f6f", null ],
      [ "pkrsrv_lobby_session_getby_table_id", "group__lobby.html#gae63742b6573e2c86a8db5142d3d888cf", null ],
      [ "pkrsrv_lobby_session_proceed__async", "group__lobby.html#ga0b5df1896257cee935abec4d155dee47", null ],
      [ "pkrsrv_lobby_session_poker_start__async", "group__lobby.html#ga6ca874293d09370bd715ec6073ac714a", null ],
      [ "pkrsrv_lobby_session_poker_restart__async", "group__lobby.html#ga648739491c2dbe6ffb391c5190d5d56a", null ],
      [ "pkrsrv_lobby_session_client_getby_socket", "group__lobby.html#ga8481b263e71d340e1540dc291198e8ac", null ],
      [ "pkrsrv_lobby_session_client_getby_account_id", "group__lobby.html#ga432d2a388a7e55c6951d0497d59ba6d3", null ],
      [ "pkrsrv_lobby_client_session_getby_session_id", "group__lobby.html#gab6b282f913667a7281eb452dbc7fa859", null ],
      [ "ref_counted", "group__lobby.html#a2a68adf15f6b617ff2927238489f60c9", null ],
      [ "next", "group__lobby.html#aebd06b4bfffc92df372a7b9018b89aab", null ],
      [ "prev", "group__lobby.html#a627d0c701cb0b6d8bd4d848d5dec2715", null ],
      [ "pg_conn", "group__lobby.html#a0e2992b29bf04de8ca0b3ef784c0b461", null ],
      [ "lobby", "group__lobby.html#a57e8193e1279dca2282c7c58d67fa4cd", null ],
      [ "id", "group__lobby.html#a1b6cd8d9be0ca6f848b83194f38b40ba", null ],
      [ "process_latency", "group__lobby.html#a0de031aed94a4b92eefbdb48dd750272", null ],
      [ "poker", "group__lobby.html#a69b511850fcb34de2cf99b63f29f5978", null ],
      [ "mutex", "group__lobby.html#ad838c6de84e96558aae8eadc5aba95e1", null ],
      [ "is_running", "group__lobby.html#a9d9565c76875b9cf0e8f1e6a104605cc", null ],
      [ "proceed_task", "group__lobby.html#a763123e5f0450b8dd3ab4c450b587e1d", null ],
      [ "poker_start_task", "group__lobby.html#abb5ae933ce57a3c5dead01c189b34ef0", null ],
      [ "poker_restart_task", "group__lobby.html#adf1c1d12ddcb597faf4548e129e1fd05", null ],
      [ "clients", "group__lobby.html#a07fd73766edb593ba49efd7f7fdc2a7e", null ],
      [ "joined_clients_count", "group__lobby.html#a54b837e1e02cb93ca2e27bd1b7c15e28", null ]
    ] ],
    [ "pkrsrv_lobby_session_clients_t", "group__lobby.html#structpkrsrv__lobby__session__clients", [
      [ "pkrsrv_lobby_session_clients_add", "group__lobby.html#gad1a06d0c2eaab3b7b89f752ad48ad065", null ],
      [ "pkrsrv_lobby_session_clients_remove", "group__lobby.html#gaf0bb39fc32b218f54c8d7d0c71ceb006", null ],
      [ "next", "group__lobby.html#a283057c2b31a7f469f2b09af2f93f8f0", null ],
      [ "prev", "group__lobby.html#a6717ee2d50b583a9388d4c380144fde9", null ],
      [ "terminal", "group__lobby.html#adddf8f6d4e92546b597c0fe02c492f05", null ],
      [ "length", "group__lobby.html#a737af09f015d0651eb03deb75bb90992", null ]
    ] ],
    [ "pkrsrv_lobby_session_client_t", "group__lobby.html#structpkrsrv__lobby__session__client", [
      [ "pkrsrv_lobby_session_client_new", "group__lobby.html#gaece3e9ea35cc49aa0d170a0d3daa9391", null ],
      [ "pkrsrv_lobby_session_client_free", "group__lobby.html#ga630c8b84a2a183142e3cc0491bd07215", null ],
      [ "next", "group__lobby.html#af41f6635c66f1bee382f4bf259df1f41", null ],
      [ "prev", "group__lobby.html#a21c608a422a16776feadbe9a0a33f738", null ],
      [ "client", "group__lobby.html#a74f0a56d9ad3dc8d8530a2eb2d35824e", null ],
      [ "is_joined", "group__lobby.html#aead577c9ad45e8ad7725f8e9efa8da9a", null ],
      [ "ref_counted", "group__lobby.html#a4f152785415ae2791f87260ffe1e92ec", null ]
    ] ],
    [ "pkrsrv_lobby_client_session_t", "group__lobby.html#structpkrsrv__lobby__client__session", [
      [ "pkrsrv_lobby_client_session_new", "group__lobby.html#gafd16f82d3d35d94744c0c45ffe51f5cc", null ],
      [ "pkrsrv_lobby_client_session_free", "group__lobby.html#gac69a555b6dcf33bedd84f3473381f51e", null ],
      [ "next", "group__lobby.html#af3297a08febc65c7275f764fd20b51e0", null ],
      [ "prev", "group__lobby.html#ae7dc61c0b9cd116eeb9d3b7c358397f6", null ],
      [ "session", "group__lobby.html#a54d8d44482a5ddc86b4b18b147e5d8d8", null ],
      [ "is_joined", "group__lobby.html#a5b768184edc99315a165ad1607afa412", null ],
      [ "ref_counted", "group__lobby.html#a2c5f9f68a4a53cce9cabaab1de551689", null ]
    ] ],
    [ "pkrsrv_lobby_client_sessions_t", "group__lobby.html#structpkrsrv__lobby__client__sessions", [
      [ "pkrsrv_lobby_client_sessions_add", "group__lobby.html#gaf964aaafa5a0431905e5881cd29ba02f", null ],
      [ "pkrsrv_lobby_client_sessions_remove", "group__lobby.html#ga345476c0325302c409a9a3198558f0c2", null ],
      [ "next", "group__lobby.html#a480cc9e34834b284feeee9c48473c7e1", null ],
      [ "prev", "group__lobby.html#ad4f6ce089c265971e0d2b4f090010963", null ],
      [ "terminal", "group__lobby.html#a84e95cabfebeb52f8da9f18af96ed81c", null ],
      [ "length", "group__lobby.html#a29c31051e55a01183d83e7e7208e3b4e", null ]
    ] ],
    [ "pkrsrv_lobby_client_t", "group__lobby.html#structpkrsrv__lobby__client", [
      [ "pkrsrv_lobby_client_enter_session", "group__lobby.html#ga7a653db08fc5607ec40500d0de849d3f", null ],
      [ "pkrsrv_lobby_client_leave_session", "group__lobby.html#ga7e3872b96f7ebbae6f9d576b47a158f0", null ],
      [ "pkrsrv_lobby_client_join_session", "group__lobby.html#ga315240f43b2ed65aa20c64124968b6a3", null ],
      [ "pkrsrv_lobby_client_unjoin_session", "group__lobby.html#ga942a5b089945f86fcd01bc3223f817d6", null ],
      [ "pkrsrv_lobby_client_get_session_by_id", "group__lobby.html#gabb0a3b91afa672d690842aefc5746b84", null ],
      [ "pkrsrv_lobby_client_session_getby_table_id", "group__lobby.html#gaaa0b9954a8b0485bf2755d8380e5ae8e", null ],
      [ "ref_counted", "group__lobby.html#ad3d43cb0530ff08d2c6683ee32a447cb", null ],
      [ "lobby", "group__lobby.html#a704d27f8b9532eb4ad32c24b4907164a", null ],
      [ "client", "group__lobby.html#af4fdcdcf2dbb6cccba6e662ae140d9fb", null ],
      [ "sessions", "group__lobby.html#a047e19d67ac2f3f65b0b013281fc4e14", null ],
      [ "account", "group__lobby.html#a0385fc95d6f3fff297300aee9c94e744", null ]
    ] ],
    [ "on_account_updated_params_t", "group__lobby.html#structon__account__updated__params", [
      [ "account", "group__lobby.html#a35d36494d43b27c03c14ac09b41f879f", null ],
      [ "lobby_client", "group__lobby.html#a17682af1bc5c0effe31ac75a604f90c2", null ]
    ] ],
    [ "pkrsrv_lobby_new_params_t", "group__lobby.html#structpkrsrv__lobby__new__params", [
      [ "port", "group__lobby.html#a1bdb3b7a390f99ff2909c372023e897f", null ],
      [ "bind_address", "group__lobby.html#ac827090ea9cd7da1ddcaa0f493fa56d4", null ],
      [ "process_latency", "group__lobby.html#aa6b33c2ad240bfad6eede10eedf76cda", null ],
      [ "max_clients", "group__lobby.html#a38f43d8ea69e50adb8cbcfcef6916d3f", null ]
    ] ],
    [ "pkrsrv_lobby_session_new_params_t", "group__lobby.html#structpkrsrv__lobby__session__new__params", [
      [ "lobby", "group__lobby.html#a3d11a00f3312489413a13d351e4efa85", null ],
      [ "table", "group__lobby.html#af0f21803bd9be7b98759da3aaa56aa99", null ]
    ] ],
    [ "pkrsrv_lobby_session_proceed__async_params_t", "group__lobby.html#structpkrsrv__lobby__session__proceed____async__params", [
      [ "session", "group__lobby.html#a48fa64d241352c55af2d6dd78272f876", null ]
    ] ],
    [ "pkrsrv_lobby_session_poker_start__async_params_t", "group__lobby.html#structpkrsrv__lobby__session__poker__start____async__params", [
      [ "session", "group__lobby.html#a0075eb58ab8c3fb6ef13558056f21314", null ]
    ] ],
    [ "pkrsrv_lobby_session_poker_restart__async_params_t", "group__lobby.html#structpkrsrv__lobby__session__poker__restart____async__params", [
      [ "session", "group__lobby.html#ad3b677cd8eea1523a5c4ed992d372723", null ]
    ] ],
    [ "on_account_updated__async", "group__lobby.html#ga22f42b0b58c5f0de9e76991c8d060241", null ],
    [ "pkrsrv_lobby_new", "group__lobby.html#gad10656320394a900881313c952c4b356", null ],
    [ "pkrsrv_lobby_free", "group__lobby.html#ga1cbf13250d8c78c9acaee8d3fccafb53", null ],
    [ "pkrsrv_lobby_run", "group__lobby.html#ga2dd4e0a417a05a452e084f937531129b", null ],
    [ "pkrsrv_lobby_client_new", "group__lobby.html#ga36db0940b3185794aaef3f3fe9d1f774", null ],
    [ "pkrsrv_lobby_client_set_account", "group__lobby.html#ga1bfa1628af3ae168c94169203ab59471", null ],
    [ "pkrsrv_lobby_client_free", "group__lobby.html#ga7c08f9f1cb1b3907bff1c72b3799241a", null ],
    [ "reorder_updated_session", "group__lobby.html#ga91a490902036e138c33b4139e49723a6", null ],
    [ "pkrsrv_lobby_broadcast_sessions", "group__lobby.html#ga10b5036d20b037af37427e6b9721663e", null ],
    [ "pkrsrv_lobby_session::pkrsrv_lobby_session_new", "group__lobby.html#ga8b101a21efde6525bb426d01737079c0", null ],
    [ "pkrsrv_lobby_session::pkrsrv_lobby_session_free", "group__lobby.html#ga5f5427dfbd357951c90f1a5a32a238e3", null ],
    [ "pkrsrv_lobby_sessions::pkrsrv_lobby_sessions_add", "group__lobby.html#gad97ec74688302a19a98490613e153c85", null ],
    [ "pkrsrv_lobby_sessions::pkrsrv_lobby_sessions_remove", "group__lobby.html#ga589399d85a561cb14067720c6b276522", null ],
    [ "pkrsrv_lobby_session::pkrsrv_lobby_session_start", "group__lobby.html#ga121deaeff46406fcc61699498efeb499", null ],
    [ "pkrsrv_lobby_session::pkrsrv_lobby_session_stop", "group__lobby.html#gaf74ed9186c5506ca9b7362828d9250ac", null ],
    [ "pkrsrv_lobby_session::pkrsrv_lobby_session_getby_id", "group__lobby.html#gaf54d795b883b8c35a3bbe5a004329f6f", null ],
    [ "pkrsrv_lobby_session::pkrsrv_lobby_session_getby_table_id", "group__lobby.html#gae63742b6573e2c86a8db5142d3d888cf", null ],
    [ "pkrsrv_lobby_session::pkrsrv_lobby_session_proceed__async", "group__lobby.html#ga0b5df1896257cee935abec4d155dee47", null ],
    [ "pkrsrv_lobby_session::pkrsrv_lobby_session_poker_start__async", "group__lobby.html#ga6ca874293d09370bd715ec6073ac714a", null ],
    [ "pkrsrv_lobby_session::pkrsrv_lobby_session_poker_restart__async", "group__lobby.html#ga648739491c2dbe6ffb391c5190d5d56a", null ],
    [ "pkrsrv_lobby_session_client::pkrsrv_lobby_session_client_new", "group__lobby.html#gaece3e9ea35cc49aa0d170a0d3daa9391", null ],
    [ "pkrsrv_lobby_session_client::pkrsrv_lobby_session_client_free", "group__lobby.html#ga630c8b84a2a183142e3cc0491bd07215", null ],
    [ "pkrsrv_lobby_session_clients::pkrsrv_lobby_session_clients_add", "group__lobby.html#gad1a06d0c2eaab3b7b89f752ad48ad065", null ],
    [ "pkrsrv_lobby_session_clients::pkrsrv_lobby_session_clients_remove", "group__lobby.html#gaf0bb39fc32b218f54c8d7d0c71ceb006", null ],
    [ "pkrsrv_lobby_client_session::pkrsrv_lobby_client_session_new", "group__lobby.html#gafd16f82d3d35d94744c0c45ffe51f5cc", null ],
    [ "pkrsrv_lobby_client_session::pkrsrv_lobby_client_session_free", "group__lobby.html#gac69a555b6dcf33bedd84f3473381f51e", null ],
    [ "pkrsrv_lobby_client_sessions::pkrsrv_lobby_client_sessions_add", "group__lobby.html#gaf964aaafa5a0431905e5881cd29ba02f", null ],
    [ "pkrsrv_lobby_client_sessions::pkrsrv_lobby_client_sessions_remove", "group__lobby.html#ga345476c0325302c409a9a3198558f0c2", null ],
    [ "pkrsrv_lobby_client::pkrsrv_lobby_client_enter_session", "group__lobby.html#ga7a653db08fc5607ec40500d0de849d3f", null ],
    [ "pkrsrv_lobby_client::pkrsrv_lobby_client_leave_session", "group__lobby.html#ga7e3872b96f7ebbae6f9d576b47a158f0", null ],
    [ "pkrsrv_lobby_client::pkrsrv_lobby_client_join_session", "group__lobby.html#ga315240f43b2ed65aa20c64124968b6a3", null ],
    [ "pkrsrv_lobby_client::pkrsrv_lobby_client_unjoin_session", "group__lobby.html#ga942a5b089945f86fcd01bc3223f817d6", null ],
    [ "pkrsrv_lobby_client::pkrsrv_lobby_client_get_session_by_id", "group__lobby.html#gabb0a3b91afa672d690842aefc5746b84", null ],
    [ "pkrsrv_lobby_client::pkrsrv_lobby_client_session_getby_table_id", "group__lobby.html#gaaa0b9954a8b0485bf2755d8380e5ae8e", null ],
    [ "pkrsrv_lobby_session::pkrsrv_lobby_session_client_getby_socket", "group__lobby.html#ga8481b263e71d340e1540dc291198e8ac", null ],
    [ "pkrsrv_lobby_session::pkrsrv_lobby_session_client_getby_account_id", "group__lobby.html#ga432d2a388a7e55c6951d0497d59ba6d3", null ],
    [ "pkrsrv_lobby_session::pkrsrv_lobby_client_session_getby_session_id", "group__lobby.html#gab6b282f913667a7281eb452dbc7fa859", null ]
];