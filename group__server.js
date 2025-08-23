var group__server =
[
    [ "pkrsrv_server_packet_login_t", "group__server.html#structpkrsrv__server__packet__login__t", [
      [ "id_token", "group__server.html#aaf9c180c61869863c7d02c2c36173bfd", null ],
      [ "password", "group__server.html#a7a83433a95b278c7f1b3986a9212577b", null ]
    ] ],
    [ "pkrsrv_server_packet_signup_t", "group__server.html#structpkrsrv__server__packet__signup__t", [
      [ "id_token", "group__server.html#a80dbf4d749a39d83eea4b08388ba474b", null ],
      [ "password", "group__server.html#acea4619cb94e2564cacf268f68f7206d", null ],
      [ "name", "group__server.html#a58aa300f7351fd90fe19a9bf0bb85a8c", null ],
      [ "avatar_length", "group__server.html#a29c181b5699100425f8d929d653dcc0d", null ],
      [ "avatar", "group__server.html#a43cea30be901bbf7f9c25e9807dbe95d", null ]
    ] ],
    [ "pkrsrv_server_packet_auth_session_t", "group__server.html#structpkrsrv__server__packet__auth__session__t", [
      [ "token", "group__server.html#a931b94078f7712792b1a70464d1dc3a7", null ]
    ] ],
    [ "pkrsrv_server_packet_enter_t", "group__server.html#structpkrsrv__server__packet__enter__t", [
      [ "table_id", "group__server.html#abe34b17504683cf90421ae02c1c2d07a", null ]
    ] ],
    [ "pkrsrv_server_packet_leave_t", "group__server.html#structpkrsrv__server__packet__leave__t", [
      [ "table_id", "group__server.html#a722ab143a2998c9cef5fa7bb97ba6151", null ]
    ] ],
    [ "pkrsrv_server_packet_join_t", "group__server.html#structpkrsrv__server__packet__join__t", [
      [ "table_id", "group__server.html#a19127530bfc0d1a2657d55ec6628c659", null ],
      [ "enterance_amount", "group__server.html#ad6cbe39ac9879a87f1ff721b3da83553", null ],
      [ "position", "group__server.html#a68a6ad1053bbf8357fc0565177a380b9", null ]
    ] ],
    [ "pkrsrv_server_packet_unjoin_t", "group__server.html#structpkrsrv__server__packet__unjoin__t", [
      [ "table_id", "group__server.html#a79ae6ad2bc44acc1cc35451cb7ae01ad", null ]
    ] ],
    [ "pkrsrv_server_packet_poker_action_t", "group__server.html#structpkrsrv__server__packet__poker__action__t", [
      [ "table_id", "group__server.html#a17e553d14c2970b3c42345b3b88fb3ae", null ],
      [ "action_kind", "group__server.html#a6334a0ee8ab2ff2d2944d4e3ce219529", null ],
      [ "amount", "group__server.html#afdc5dae04196d6ab33a73e33f7dc85ba", null ]
    ] ],
    [ "pkrsrv_server_packet_get_tables_t", "group__server.html#structpkrsrv__server__packet__get__tables__t", [
      [ "offset", "group__server.html#a72311278528000a8a091cdfb771f82ba", null ],
      [ "length", "group__server.html#a1dff03f0b03536e6db64c89059e454ae", null ]
    ] ],
    [ "pkrsrv_server_packet_get_sessions_t", "group__server.html#structpkrsrv__server__packet__get__sessions__t", [
      [ "offset", "group__server.html#a87da66b6bf0838813959af9b8d2f8a6f", null ],
      [ "length", "group__server.html#ae308fff416eca51b64477cc31962813e", null ]
    ] ],
    [ "pkrsrv_server_packet_json_t", "group__server.html#structpkrsrv__server__packet__json__t", [
      [ "json", "group__server.html#ab63452dbd69977c294d8ef6bfd080354", null ]
    ] ],
    [ "pkrsrv_server_packet_update_account_t", "group__server.html#structpkrsrv__server__packet__update__account__t", [
      [ "name", "group__server.html#a1f5e82e002839ae694eca9cd482871a0", null ],
      [ "avatar", "group__server.html#a27059c8f16b91466b1f6a92bff8c18d6", null ]
    ] ],
    [ "on_client_connected_params_t", "group__server.html#structon__client__connected__params__t", [
      [ "owner", "group__server.html#ad6376424da66c825b08954272b5b2ae0", null ],
      [ "client", "group__server.html#ac1be996e7cd3a6b6045f9f58e972c263", null ]
    ] ],
    [ "on_client_disconnected_params_t", "group__server.html#structon__client__disconnected__params__t", [
      [ "owner", "group__server.html#a633d2d06fd6351d5f84a5dbfb6eeb904", null ],
      [ "client", "group__server.html#a58eba5d1f08a8c9411ad2d1d43beed42", null ]
    ] ],
    [ "on_client_meowed_params_t", "group__server.html#structon__client__meowed__params__t", [
      [ "owner", "group__server.html#ab7b157f408c4ae75dc02f0904c06a217", null ],
      [ "client", "group__server.html#a053e37db0fa1873107e0faefc7c94198", null ]
    ] ],
    [ "on_client_enter_params_t", "group__server.html#structon__client__enter__params__t", [
      [ "owner", "group__server.html#aa511648e72192cf8d1e9f264fca19002", null ],
      [ "client", "group__server.html#a044e7662ffe76db7c21a064197fe77ee", null ],
      [ "enter", "group__server.html#a21c41613a323810274c779f3a7f5fe0a", null ]
    ] ],
    [ "on_client_leave_params_t", "group__server.html#structon__client__leave__params__t", [
      [ "owner", "group__server.html#a378c70dc26d9cfc9e0ed964818061de7", null ],
      [ "client", "group__server.html#a2825d7c4cc985f7dc817916fe50d3753", null ],
      [ "leave", "group__server.html#a258429879f2d21b707df57e07935192e", null ]
    ] ],
    [ "on_client_join_params_t", "group__server.html#structon__client__join__params__t", [
      [ "owner", "group__server.html#a15c8337a09f16ffc891bf234bfcd1e4f", null ],
      [ "client", "group__server.html#ae96431d4644a73a1752a877224fedf58", null ],
      [ "join", "group__server.html#ae2875f3c924071947329c252d82dc60a", null ]
    ] ],
    [ "on_client_unjoin_params_t", "group__server.html#structon__client__unjoin__params__t", [
      [ "owner", "group__server.html#aefc682815b65a3075e0dcb12690283bd", null ],
      [ "client", "group__server.html#a980fa3ce3f5d950f0e355a52b48afca6", null ],
      [ "unjoin", "group__server.html#a0111bad1bf6984f766a14de96dc99ce1", null ]
    ] ],
    [ "on_client_login_params_t", "group__server.html#structon__client__login__params__t", [
      [ "owner", "group__server.html#a6b8edca6bf4fac887cfd28498f9a7d80", null ],
      [ "client", "group__server.html#ae96df57772514ce5eca35ed7f09c3cd8", null ],
      [ "login", "group__server.html#a34ceebb44f233336c689cdcc17c14dc6", null ]
    ] ],
    [ "on_client_signup_params_t", "group__server.html#structon__client__signup__params__t", [
      [ "owner", "group__server.html#a232378ae700214756756c8380795c7ff", null ],
      [ "client", "group__server.html#a1dcbc4a9a00186002eaf45d650193065", null ],
      [ "signup", "group__server.html#a26f128c73b48cba1afef11fdc3197675", null ]
    ] ],
    [ "on_client_auth_session_params_t", "group__server.html#structon__client__auth__session__params__t", [
      [ "owner", "group__server.html#ace3f781e80e567c1c6b03d70a887a4db", null ],
      [ "client", "group__server.html#ad63655f7aff861769515a659cef92f50", null ],
      [ "auth_session", "group__server.html#a1cf41c0a458aedf9c0ab483a6db6fa8d", null ]
    ] ],
    [ "on_client_get_account_params_t", "group__server.html#structon__client__get__account__params__t", [
      [ "owner", "group__server.html#af3f1516b387bf7c88fbc14408541ccde", null ],
      [ "client", "group__server.html#aa2e031765611a3976caae639bbbb0095", null ]
    ] ],
    [ "on_client_action_params_t", "group__server.html#structon__client__action__params__t", [
      [ "owner", "group__server.html#ab47d9fbe40eda36808be53620a0c51f5", null ],
      [ "client", "group__server.html#a0e6edcff2f4276e6509898240117e8ed", null ],
      [ "action", "group__server.html#afdcffd18a3ce54add579ba4d435b7455", null ]
    ] ],
    [ "on_client_get_tables_params_t", "group__server.html#structon__client__get__tables__params__t", [
      [ "owner", "group__server.html#ab40c35fbde060e5f0159a0bde19f7c95", null ],
      [ "client", "group__server.html#a6e1edcb6fe98b246e23c71db778a5cc0", null ],
      [ "get_tables", "group__server.html#a2467611dcd97f83d0f4b00a90ff8eb3a", null ]
    ] ],
    [ "on_client_get_sessions_params_t", "group__server.html#structon__client__get__sessions__params__t", [
      [ "owner", "group__server.html#afea5f3dfc0a42dc564272981fdb5748d", null ],
      [ "client", "group__server.html#a4f4375d297fff1876cb10871552a4698", null ],
      [ "get_sessions", "group__server.html#a17e3ea1af4c8963e176c1f68caf2fe89", null ]
    ] ],
    [ "on_client_update_account_params_t", "group__server.html#structon__client__update__account__params__t", [
      [ "owner", "group__server.html#acbc64bd8d571ea173e72cd7345f434d5", null ],
      [ "client", "group__server.html#aea61107d63402c35eae73f70cddb92af", null ],
      [ "update_account", "group__server.html#a005bf3a29357e7f48e04053378af359c", null ]
    ] ],
    [ "on_client_json_params_t", "group__server.html#structon__client__json__params__t", [
      [ "owner", "group__server.html#ab6bc244b695b4cfd9a54832599231c88", null ],
      [ "client", "group__server.html#a4b5cef1dadc465a19fa51901ac1db032", null ],
      [ "json", "group__server.html#a9076ed662dec1e9828ae763a621306ff", null ]
    ] ],
    [ "pkrsrv_server_t", "group__server.html#structpkrsrv__server", [
      [ "pkrsrv_server_new", "group__server.html#gad13cceb5dc89f70a8c77d7aeb0b473e6", null ],
      [ "pkrsrv_server_start", "group__server.html#gac3ce3288aea013e27a7b505bf116c1c9", null ],
      [ "pkrsrv_server_listen", "group__server.html#gab6d1fae9c8487d9a6afb4589ba2db6c0", null ],
      [ "pkrsrv_server_free", "group__server.html#gafcdb70f08e80d4a37abc466c835fe95f", null ],
      [ "ref_counted", "group__server.html#ab605ffe0a8db4967c53da418ee1c1644", null ],
      [ "owner", "group__server.html#aa444f5a0d815a069440ac6460ea851d9", null ],
      [ "eventloop", "group__server.html#ac1e51e8dad2e4c07aaa0dd432e11969b", null ],
      [ "port", "group__server.html#a807ef94ef19f9e3e29a43f488841395d", null ],
      [ "bind_address", "group__server.html#ab2343287b9d189aad408d5d229c9e9b9", null ],
      [ "host_address", "group__server.html#a48fee688119bd3e0b8c1b0b4725fae47", null ],
      [ "max_clients", "group__server.html#af3980458dd2a32174d7915708e026115", null ],
      [ "clients", "group__server.html#a0ee4ce34ee7e2ed4b2603c459636466d", null ],
      [ "ssl_ctx", "group__server.html#afaf3deb6d336b14e25a893da4ecd0b35", null ],
      [ "on_client_connected", "group__server.html#abaf317b5371049c44a78252f992b62b9", null ],
      [ "on_client_disconnected", "group__server.html#ab1e7bb4a72b11f9952ac9bd9b25e67ed", null ],
      [ "on_client_meowed", "group__server.html#a70012e10271a01751cb9ddc7f9b79a39", null ],
      [ "on_client_login", "group__server.html#afe73b794e1f45c32371dcb5933e0d69f", null ],
      [ "on_client_signup", "group__server.html#a4a667e0d09b474a9f7f01f3cd1f88e56", null ],
      [ "on_client_auth_session", "group__server.html#a2787bf659edcba1cd5a3ca53132007e9", null ],
      [ "on_client_get_account", "group__server.html#a45bcb6d593d4242c6cf1740ba04a4e38", null ],
      [ "on_client_enter", "group__server.html#a91893f41e8e432542c0b21c17d6d3a86", null ],
      [ "on_client_leave", "group__server.html#aedea840f3f4d8032265b092e7fecc834", null ],
      [ "on_client_join", "group__server.html#a7c9d1f1677c551b66b6f090dbfb130d0", null ],
      [ "on_client_unjoin", "group__server.html#ac5e6e542123614154cc34e60478ea174", null ],
      [ "on_client_action", "group__server.html#aaa138b5f11c37755e99af3c4c06510c4", null ],
      [ "on_client_get_tables", "group__server.html#a7ed81d5b7d03d6ef9cb0ff52ecf5bcd1", null ],
      [ "on_client_get_sessions", "group__server.html#a999d50229bb7fe0a19bde331e24c4c51", null ],
      [ "on_client_update_account", "group__server.html#aec49b058203480eb55c0ab9e8c4c43f1", null ],
      [ "on_client_json", "group__server.html#a148e11685130d90f63e8ff9e2cb5e8bc", null ],
      [ "mutex", "group__server.html#ae5ccd6edcd2aa44f3d5d646b0204e2ea", null ],
      [ "thread", "group__server.html#ab6d83852e1a86a9fa65770480eae0592", null ],
      [ "is_running", "group__server.html#a68ea2608cf3743b2710145b408a79e68", null ]
    ] ],
    [ "pkrsrv_server_clients_t", "group__server.html#structpkrsrv__server__clients", [
      [ "prev", "group__server.html#aad85a7aae8b6e3bb3cf61e4b78ef2092", null ],
      [ "next", "group__server.html#a1a6af8ad1a7142692c0723b6b2616b51", null ],
      [ "terminal", "group__server.html#a597c335f6af65b03a840d1b05f9c9622", null ],
      [ "length", "group__server.html#a9c21da0eca214ba8b49c3ce8fb60d626", null ]
    ] ],
    [ "pkrsrv_server_client_t", "group__server.html#structpkrsrv__server__client", [
      [ "ref_counted", "group__server.html#a8855c1d94d8bd948d8af58d20e9fb107", null ],
      [ "next", "group__server.html#abe54f9910c7def27088ee5eb65f10b9d", null ],
      [ "prev", "group__server.html#a368d7dfdb749c9170ac9eaac7440e0ca", null ],
      [ "server", "group__server.html#ac56bb3faf38d41e4693f98c2189f6167", null ],
      [ "pg_conn", "group__server.html#a3da156ff85222adc932458bcc7cbb194", null ],
      [ "ssl", "group__server.html#ab6fc5f3c00b5200c0fbd91309e587d8f", null ],
      [ "is_alive", "group__server.html#a9a98cc511596c136c89fe3a85d0a523c", null ],
      [ "is_protocol_determined", "group__server.html#a79f84b22d70c46145eecd966c0c20ec8", null ],
      [ "is_websocket", "group__server.html#affacc494a8e672408e99122bc13b00a3", null ],
      [ "websocket", "group__server.html#af93c03a9a83ceafac0543b40f2916e83", null ],
      [ "write_mutex", "group__server.html#a31f080000a4c657b00f07372f5f47ebb", null ],
      [ "socket", "group__server.html#a8a69763b51f79e7d4c1b77167a015baa", null ],
      [ "server_socket", "group__server.html#a938c60430bd010bd1fa1e51334a1b6dc", null ],
      [ "address", "group__server.html#a3a0ec9cd91bb07c2378f3b014a557aa6", null ],
      [ "owner", "group__server.html#a6acbb56a892eceefa411922ecd75b2be", null ]
    ] ],
    [ "pkrsrv_server_new_params_t", "group__server.html#structpkrsrv__server__new__params", [
      [ "owner", "group__server.html#a41ad647c6503559a8d6d0e7d5e974649", null ],
      [ "port", "group__server.html#a9a494e3e6597c37b1ed8e2c3b1859ec0", null ],
      [ "bind_address", "group__server.html#aec96cd2d0c4a76b6dd093ed55faa99da", null ],
      [ "eventloop", "group__server.html#abeeecb9aff92ceed2f02c8fbea629386", null ],
      [ "max_clients", "group__server.html#a899e3388956062ebf4c8af83318130e7", null ],
      [ "on_client_connected", "group__server.html#a94e76d046507767f2bdd59fb1988a487", null ],
      [ "on_client_disconnected", "group__server.html#acd7e9f390bf6e4085efc1bd2fd67dd94", null ],
      [ "on_client_meowed", "group__server.html#ad70a60e029024b5ea3806b00f9dc6d01", null ],
      [ "on_client_login", "group__server.html#a695469e226f1e2efa3df9d2729ba7466", null ],
      [ "on_client_signup", "group__server.html#a0042f1c88647d4c61601c7f38a5d2296", null ],
      [ "on_client_auth_session", "group__server.html#a29df9e6cab379bcc80257b0a352bb7c9", null ],
      [ "on_client_get_account", "group__server.html#a966bfe22ba2fed72f2b82f34cf1410f1", null ],
      [ "on_client_enter", "group__server.html#a0be82ae7766f4023fd74f0a31362b1e1", null ],
      [ "on_client_leave", "group__server.html#a24939fbc6bed655ec6d79b16724d51e2", null ],
      [ "on_client_join", "group__server.html#a9cce86f2c4e8dbd3a886826dfc933509", null ],
      [ "on_client_unjoin", "group__server.html#ac876ee1930322aa31d17971f69162b4c", null ],
      [ "on_client_action", "group__server.html#a6093f48fbf047d451aead35fbf36eaf1", null ],
      [ "on_client_get_tables", "group__server.html#ab98387e038946b9520a25b5594c794d5", null ],
      [ "on_client_get_sessions", "group__server.html#a12ace6652a64906ef80850beed48e9d0", null ],
      [ "on_client_update_account", "group__server.html#a6b7fc2d3b8efb777b5f907eff2e3b343", null ],
      [ "on_client_json", "group__server.html#a40619c80f5b1c6de24a17a350ceea66f", null ]
    ] ],
    [ "pkrsrv_server_send_login_res_params_t", "group__server.html#structpkrsrv__server__send__login__res__params__t", [
      [ "client", "group__server.html#a0786edb700e55bdd5050a53592957e34", null ],
      [ "is_ok", "group__server.html#aba7384faf944daa4f2cd39f008802a59", null ],
      [ "is_logined", "group__server.html#a7919c0f0cc4d6cb831fabae0fd77bd89", null ],
      [ "account", "group__server.html#a32c3cbdac8e8c73b92aa8b108d85b07f", null ],
      [ "auth_session", "group__server.html#a78c74e08e879e149ffd16922c2fc3198", null ]
    ] ],
    [ "pkrsrv_server_send_signup_res_params_t", "group__server.html#structpkrsrv__server__send__signup__res__params__t", [
      [ "client", "group__server.html#a0a7244a6b11ddc8d52309b747f242c78", null ],
      [ "is_ok", "group__server.html#ae1d27cb860a515cabc3d4cc620676ed6", null ],
      [ "is_logined", "group__server.html#aa578213fcf621e15cc0592a31ff0594f", null ],
      [ "status", "group__server.html#af17086ec49ac7fdf32c71f887a8fa89f", null ],
      [ "account", "group__server.html#a6aae095f86cb201de480a6416f978a9a", null ],
      [ "auth_session", "group__server.html#a7541b7f486a92f0caa8f19b962f6d893", null ]
    ] ],
    [ "pkrsrv_server_send_auth_session_res_params_t", "group__server.html#structpkrsrv__server__send__auth__session__res__params__t", [
      [ "client", "group__server.html#a0366fabca280c837da80647e8eb4755c", null ],
      [ "is_ok", "group__server.html#a1120f0daa74d6095f0a8451e91dff4be", null ],
      [ "is_logined", "group__server.html#ac6af5f4da8efecc18eea7049e1940d5f", null ],
      [ "account", "group__server.html#a3b8194705b71b79f71c98e5c0c7dd363", null ]
    ] ],
    [ "pkrsrv_server_send_account_params_t", "group__server.html#structpkrsrv__server__send__account__params__t", [
      [ "client", "group__server.html#a75a678153b52b057497d8693742e0556", null ],
      [ "account", "group__server.html#a0a9a51e0e8252582f71ba6d7e9103048", null ]
    ] ],
    [ "pkrsrv_server_send_enter_res_params_t", "group__server.html#structpkrsrv__server__send__enter__res__params__t", [
      [ "client", "group__server.html#a4aefa7088fdda72238cf8c92b897acd2", null ],
      [ "table_id", "group__server.html#a7fcf3af378ea57c256c703dc1a4402b8", null ],
      [ "is_ok", "group__server.html#a63bc174388490e9dfa210b9dad32df5d", null ]
    ] ],
    [ "pkrsrv_server_send_leave_res_params_t", "group__server.html#structpkrsrv__server__send__leave__res__params__t", [
      [ "client", "group__server.html#aa37f62bf57ac25600dbecfe4b1e71508", null ],
      [ "table_id", "group__server.html#ac8f7f06fa9c8ce71d1f5fd3978a77fa2", null ],
      [ "is_ok", "group__server.html#ae49ca9eab2c47f9b6b2d4b575459d438", null ]
    ] ],
    [ "pkrsrv_server_send_join_res_params_t", "group__server.html#structpkrsrv__server__send__join__res__params__t", [
      [ "client", "group__server.html#ae7f96390df187a822e6fef06539d82ad", null ],
      [ "table_id", "group__server.html#ad679e632905f7d03bae29c842a762779", null ],
      [ "is_ok", "group__server.html#af9b32f208800efce6a679200aa1676e2", null ]
    ] ],
    [ "pkrsrv_server_send_unjoin_res_params_t", "group__server.html#structpkrsrv__server__send__unjoin__res__params__t", [
      [ "client", "group__server.html#aff255f69bb9f493f5cf481e470d6fd5d", null ],
      [ "table_id", "group__server.html#accbf5ce8357f8e3cead854c8444d77da", null ],
      [ "is_ok", "group__server.html#a0fed4a081aca9a28936a26a3c700d5d2", null ]
    ] ],
    [ "pkrsrv_server_send_poker_info_params_t", "group__server.html#structpkrsrv__server__send__poker__info__params__t", [
      [ "client", "group__server.html#a5bb987b7cb2f3c3496465371c28d4a96", null ],
      [ "poker", "group__server.html#ad8d5e1e30229eab3448b8acc4c29846f", null ]
    ] ],
    [ "pkrsrv_server_send_poker_state_params_t", "group__server.html#structpkrsrv__server__send__poker__state__params__t", [
      [ "client", "group__server.html#a5cfca0cbce1a2c759a68fc0cb20255ec", null ],
      [ "poker", "group__server.html#ab44ffef4ae333e765964ef87328f367a", null ],
      [ "player", "group__server.html#ad1f6c2ab7193c941f145779c2050a377", null ]
    ] ],
    [ "pkrsrv_server_send_poker_action_reflection_params_t", "group__server.html#structpkrsrv__server__send__poker__action__reflection__params__t", [
      [ "client", "group__server.html#abfb45a4a7062829e6ec8518f3bc92f63", null ],
      [ "table_id", "group__server.html#a069d14af653af6a50439acb19846a94c", null ],
      [ "account_id", "group__server.html#ad00ef4584ea963ce6550fbcfcdb14e8b", null ],
      [ "action_kind", "group__server.html#abb4343ed4b06717add80bdd2dd359d4d", null ],
      [ "amount", "group__server.html#af01ef473f881f39f24e45df567a430d0", null ]
    ] ],
    [ "pkrsrv_server_send_poker_end_params_t", "group__server.html#structpkrsrv__server__send__poker__end__params__t", [
      [ "client", "group__server.html#a8f934266a7fa2b6729a60834e8da5e43", null ],
      [ "table_id", "group__server.html#a6bfa47b3b587007c763cb12e0bb5774a", null ],
      [ "winner_account_id", "group__server.html#a65a8db08d333a005a25d0368ab15edb1", null ],
      [ "earned_amount", "group__server.html#a8d7234a378830a67dc378f9af7e43f3a", null ],
      [ "poker", "group__server.html#a4425db0cd1bbd0a33add97bb7159d4d5", null ]
    ] ],
    [ "pkrsrv_server_send_poker_restarted_params_t", "group__server.html#structpkrsrv__server__send__poker__restarted__params__t", [
      [ "client", "group__server.html#ade2559af8ddbdde2961e203c6c826564", null ],
      [ "table_id", "group__server.html#ae13b4f41c7362a29eccdf7504af01888", null ]
    ] ],
    [ "pkrsrv_server_send_unjoined_params_t", "group__server.html#structpkrsrv__server__send__unjoined__params__t", [
      [ "client", "group__server.html#a469788e331e1f95585dd8d7d45590250", null ],
      [ "table_id", "group__server.html#afb4cdf9e3ee849ca7431449cb94f0fab", null ],
      [ "reason", "group__server.html#a818de0802c2c44f4d698c2d0825e83c9", null ]
    ] ],
    [ "pkrsrv_server_send_tables_params_t", "group__server.html#structpkrsrv__server__send__tables__params__t", [
      [ "client", "group__server.html#a7b95dcb0f4882f586442454f2a20e47c", null ],
      [ "offset", "group__server.html#ad511e0a94e1fa52a4ab296089c9bac93", null ],
      [ "list", "group__server.html#a497d3e89a2a33694d43143544b87ac04", null ]
    ] ],
    [ "pkrsrv_server_send_sessions_params_t", "group__server.html#structpkrsrv__server__send__sessions__params__t", [
      [ "client", "group__server.html#a7867217c887cfe03d3dfe50e96155ba8", null ],
      [ "offset", "group__server.html#aff80e951076660d855d260dc41ee2c67", null ],
      [ "pokers_length", "group__server.html#aa7cc21ac5dac62430ce1f326715c2827", null ],
      [ "pokers", "group__server.html#a10b11313a3f397f7f8d17f8d514da1c5", null ]
    ] ],
    [ "pkrsrv_server_send_update_account_res_params_t", "group__server.html#structpkrsrv__server__send__update__account__res__params__t", [
      [ "client", "group__server.html#a10a1516f122c31aae8d00f8f2aaec604", null ],
      [ "is_ok", "group__server.html#ae84c1ce2335248194857773e03d80dbd", null ],
      [ "is_avatar_too_big", "group__server.html#a0851e0400826f1b788faaa3abbdb3125", null ]
    ] ],
    [ "pkrsrv_server_send_server_info_params_t", "group__server.html#structpkrsrv__server__send__server__info__params__t", [
      [ "client", "group__server.html#a18a8c15cff224a7df890169784031422", null ],
      [ "build_number", "group__server.html#a4d10cf25a22546ce0e84cd205ea369ba", null ],
      [ "version", "group__server.html#ac8f45e094f9a3d6d3b0ff513a798b50c", null ],
      [ "revision", "group__server.html#abede4191c8af25ebd3b5182e080d947c", null ],
      [ "compiler", "group__server.html#a358442cb70f8db8f06fd76a3f2ebfaad", null ]
    ] ],
    [ "pkrsrv_server_send_json_params_t", "group__server.html#structpkrsrv__server__send__json__params__t", [
      [ "client", "group__server.html#a087e2521aeec98ec8fd072ba153d41a5", null ],
      [ "json", "group__server.html#a07624c3441299dfaf982c24dc388d263", null ]
    ] ],
    [ "pkrsrv_server_packet_frame_header_t", "group__server.html#ga2f47384263613465685ad61a329127dd", null ],
    [ "pkrsrv_server_packet_frame_login_t", "group__server.html#ga5c159ec9727fc18200def1dd0e45c2f4", null ],
    [ "pkrsrv_server_packet_frame_login_res_t", "group__server.html#ga04d8a9ff7d8f5a30de06061f05e5f3b1", null ],
    [ "pkrsrv_server_packet_frame_login_res_account_t", "group__server.html#ga2603af6d97be7a7c9a76c72392ef0a09", null ],
    [ "pkrsrv_server_packet_frame_signup_t", "group__server.html#ga22fb0b1d28382baff845f458708aaeaa", null ],
    [ "pkrsrv_server_packet_frame_signup_res_t", "group__server.html#ga59d9ff66450449b0b509ce5160bc5af8", null ],
    [ "pkrsrv_server_packet_frame_signup_res_account_t", "group__server.html#ga2383666b15ba393aef1605cef746983f", null ],
    [ "pkrsrv_server_packet_frame_auth_session_t", "group__server.html#gac53852b452fce474f24727ba60b0b485", null ],
    [ "pkrsrv_server_packet_frame_auth_session_res_t", "group__server.html#ga5f99e96ffc8993894ee48f45ff540e31", null ],
    [ "pkrsrv_server_packet_frame_auth_session_res_account_t", "group__server.html#gaea04bad397b9be0ace3f60cf4e6d7326", null ],
    [ "pkrsrv_server_packet_frame_account_t", "group__server.html#ga7963ca76f4c9260cef35e356eedbce75", null ],
    [ "pkrsrv_server_packet_frame_enter_t", "group__server.html#ga44de1e914640bbe2fd7e76a678415a4f", null ],
    [ "pkrsrv_server_packet_frame_enter_res_t", "group__server.html#ga5060bb343c459a50f41e0bb47143432f", null ],
    [ "pkrsrv_server_packet_frame_leave_t", "group__server.html#ga468a51cd59a3ac23ccb35faa3222e0b1", null ],
    [ "pkrsrv_server_packet_frame_leave_res_t", "group__server.html#ga569edbd27d9aa0025896565a94010e7c", null ],
    [ "pkrsrv_server_packet_frame_join_t", "group__server.html#ga53febf4eebe56fde1efb68378cc099a8", null ],
    [ "pkrsrv_server_packet_frame_join_res_t", "group__server.html#gab3141be8aed2649d5193bb04237241fe", null ],
    [ "pkrsrv_server_packet_frame_unjoin_t", "group__server.html#ga7e4f2d6826cdadb620dd1619efa77069", null ],
    [ "pkrsrv_server_packet_frame_unjoin_res_t", "group__server.html#ga8022d48ada618305e9d86c8fa1bc5058", null ],
    [ "pkrsrv_server_packet_frame_poker_info_t", "group__server.html#ga08a7740d2374568d574e6c2bc8a22393", null ],
    [ "pkrsrv_server_packet_frame_poker_info_player_t", "group__server.html#gac853ce81a2323fdcf2b0bc6aa675c12d", null ],
    [ "pkrsrv_server_packet_frame_poker_state_t", "group__server.html#gaad6f552cddb54c4ec70d9d173cc2972c", null ],
    [ "pkrsrv_server_packet_frame_poker_state_player_t", "group__server.html#ga0ed6f4303d4037a96107b923fb563a04", null ],
    [ "pkrsrv_server_packet_frame_poker_action_t", "group__server.html#ga53753ce56f8f9c358fa80f43b83785e2", null ],
    [ "pkrsrv_server_packet_frame_poker_action_reflection_t", "group__server.html#ga7ccdfbb03ebce3ca25761b2c9370309a", null ],
    [ "pkrsrv_server_packet_frame_poker_end_t", "group__server.html#ga2a4b3f2a3294bccbc29f3f2b91e3a84c", null ],
    [ "pkrsrv_server_packet_frame_poker_end_score_t", "group__server.html#ga83748a1900f6d9985050b887c36ec15d", null ],
    [ "pkrsrv_server_packet_frame_poker_restarted_t", "group__server.html#ga03e107f2a48726682fbd470a283edd84", null ],
    [ "pkrsrv_server_packet_frame_unjoined_t", "group__server.html#ga14ba9278b8d5ba822a968e4e4add57ca", null ],
    [ "pkrsrv_server_packet_frame_get_tables_t", "group__server.html#gadafe62e1653f42d8c50475ad7bb68b34", null ],
    [ "pkrsrv_server_packet_frame_tables_t", "group__server.html#ga421868921707a69ccb2a790910ae72cd", null ],
    [ "pkrsrv_server_packet_frame_get_sessions_t", "group__server.html#gadc6dde674644a32604e4f384be6da2a5", null ],
    [ "pkrsrv_server_packet_frame_sessions_t", "group__server.html#ga4fc34f2e88f752a096a6726589976593", null ],
    [ "pkrsrv_server_packet_frame_table_t", "group__server.html#ga1b14d1486d3757307923852ecfce313f", null ],
    [ "pkrsrv_server_packet_frame_update_account_t", "group__server.html#gac3ca78c48fe87ea023660d7678dc4e16", null ],
    [ "pkrsrv_server_packet_frame_update_account_res_t", "group__server.html#gaf5a2019723278039730c6f52bbe5e1bb", null ],
    [ "pkrsrv_server_packet_frame_server_info_t", "group__server.html#gace399e462b27f7744b55596745af76e3", null ],
    [ "pkrsrv_server_opcode_t", "group__server.html#ga8e565e44f78541e2b023c1605a55752c", null ],
    [ "opcode_handler_t", "group__server.html#gacffc655f13bd77b828446511d53fa5e1", null ],
    [ "PKRSRV_SERVER_OPCODE", "group__server.html#ga0467c2dd3419b4f933fbe11c9468f629", [
      [ "PKRSRV_SERVER_OPCODE_NOP", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629abeaccc93f9f7abf3537be99a35e4faec", null ],
      [ "PKRSRV_SERVER_OPCODE_MEOW", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629ad3b4ad4dbc7b563c4bcccb3cd8c047ad", null ],
      [ "PKRSRV_SERVER_OPCODE_PING", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629a21119837d120aed054c54e0c117fe5f3", null ],
      [ "PKRSRV_SERVER_OPCODE_PONG", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629a3632631ed3ef8823250736bebca4c066", null ],
      [ "PKRSRV_SERVER_OPCODE_LOGIN", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629a98a1d2e5143ad3ab51d17f20cf7e6b10", null ],
      [ "PKRSRV_SERVER_OPCODE_LOGIN_RES", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629aa03b2f1c9dd2d27acc32e4827d64b38c", null ],
      [ "PKRSRV_SERVER_OPCODE_SIGNUP", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629ab0fc6d1f90fb0abc7ebe3d6b2dcfafa0", null ],
      [ "PKRSRV_SERVER_OPCODE_SIGNUP_RES", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629a930575da97b3b90979bf95d184c3aabf", null ],
      [ "PKRSRV_SERVER_OPCODE_AUTH_SESSION", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629a87962759fda987cadc56e5c4b5754cbb", null ],
      [ "PKRSRV_SERVER_OPCODE_AUTH_SESSION_RES", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629aff41b014581a645815c1f1dc47cbf3ed", null ],
      [ "PKRSRV_SERVER_OPCODE_GET_ACCOUNT", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629a994c408196af7b8b6a82604a4bcf682d", null ],
      [ "PKRSRV_SERVER_OPCODE_ACCOUNT", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629a36498feac900851a1f199a49bf972766", null ],
      [ "PKRSRV_SERVER_OPCODE_ENTER", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629ac5c607d402d2f114229565084cf8ccf2", null ],
      [ "PKRSRV_SERVER_OPCODE_ENTER_RES", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629ae4828ed98fcb1986918d425d6fda199c", null ],
      [ "PKRSRV_SERVER_OPCODE_LEAVE", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629a9c462a486c2fec28f0befa8ba679ed33", null ],
      [ "PKRSRV_SERVER_OPCODE_LEAVE_RES", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629ac917e0432e4019ee29d26477a0878114", null ],
      [ "PKRSRV_SERVER_OPCODE_JOIN", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629ae543af3ab126557b6cfce86f7b169c1b", null ],
      [ "PKRSRV_SERVER_OPCODE_JOIN_RES", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629a25cafd76833225093e7a27d6ae6e4bec", null ],
      [ "PKRSRV_SERVER_OPCODE_UNJOIN", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629ac16f81aa4a1e35a2afac4d03b0c562a4", null ],
      [ "PKRSRV_SERVER_OPCODE_UNJOIN_RES", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629aa41e6b01cf8beee60c52becdf4112c7c", null ],
      [ "PKRSRV_SERVER_OPCODE_POKER_INFO", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629a9c23a42a3fb6a0d63cdcddf7c5acc925", null ],
      [ "PKRSRV_SERVER_OPCODE_POKER_STATE", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629a7347756c9c64d14650fcf23e78dd742a", null ],
      [ "PKRSRV_SERVER_OPCODE_POKER_ACTION", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629a0800adbba7bd24287b11aa5b8d8568c3", null ],
      [ "PKRSRV_SERVER_OPCODE_POKER_ACTION_REFLECTION", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629a41a1144af54f83a9518547256aca2300", null ],
      [ "PKRSRV_SERVER_OPCODE_POKER_END", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629a41a1debc7a3ab4df5b9142787c11b817", null ],
      [ "PKRSRV_SERVER_OPCODE_POKER_RESTARTED", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629a800c8fd61307741d49b0a6dcc175775a", null ],
      [ "PKRSRV_SERVER_OPCODE_JSON", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629a574e0f1c0577cce863cca7efe6254030", null ],
      [ "PKRSRV_SERVER_OPCODE_UNJOINED", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629ac9564085437e561d9bb1c85421862d3f", null ],
      [ "PKRSRV_SERVER_OPCODE_GET_TABLES", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629a5ffb2b9f0e3d3c9b1c34775744258ed4", null ],
      [ "PKRSRV_SERVER_OPCODE_TABLES", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629ad67a5a5b87af1c9efc2702c373533468", null ],
      [ "PKRSRV_SERVER_OPCODE_GET_SESSIONS", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629a2f4c9f02464e4fb455f73e5e12ecd387", null ],
      [ "PKRSRV_SERVER_OPCODE_SESSIONS", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629a5fc01ed00eeefdf2a05629cac05c7a60", null ],
      [ "PKRSRV_SERVER_OPCODE_UPDATE_ACCOUNT", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629ad313e47921fcf64e3650010e0b7d1e4f", null ],
      [ "PKRSRV_SERVER_OPCODE_UPDATE_ACCOUNT_RES", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629af5e8adb6f9d7e69ef26aa5e91fd83868", null ],
      [ "PKRSRV_SERVER_OPCODE_SERVER_INFO", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629a34ce645d35d845be7674fae6762bc22c", null ],
      [ "PKRSRV_SERVER_OPCODE_OVER_CAPACITY", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629a32727cf27e3903d80f54f48f9951b288", null ],
      [ "PKRSRV_SERVER_OPCODE_END", "group__server.html#gga0467c2dd3419b4f933fbe11c9468f629a688f0e988b7c91e27244008beecc2e6c", null ]
    ] ],
    [ "pkrsrv_server_packet_signup_res_status_t", "group__server.html#ga282513609fff2167c91230efea12cdfd", [
      [ "PKRSRV_SERVER_PACKET_SIGNUP_RES_STATUS_OK", "group__server.html#gga282513609fff2167c91230efea12cdfdad53ea6d75eee5df9e1900c5ceea64bcc", null ],
      [ "PKRSRV_SERVER_PACKET_SIGNUP_RES_STATUS_ERROR", "group__server.html#gga282513609fff2167c91230efea12cdfdaf510e68034863a55c56d41a4e05850e8", null ],
      [ "PKRSRV_SERVER_PACKET_SIGNUP_RES_STATUS_ALREADY_EXISTS", "group__server.html#gga282513609fff2167c91230efea12cdfda1be0290a387b66a68fd406ee1318f9fb", null ]
    ] ],
    [ "__attribute__", "group__server.html#ga478b6a736a949f51012ecf2067745df3", null ],
    [ "pkrsrv_server_ssl_read", "group__server.html#ga8b0339ffe352783215aeaee4fb2d7cc0", null ],
    [ "pkrsrv_server_ssl_write", "group__server.html#ga896ff5af3e02a3ac28f82bbdebad2cc8", null ],
    [ "pkrsrv_server_net_read", "group__server.html#gad835aff1bedeaeba300de1e75d71335c", null ],
    [ "pkrsrv_server_net_write", "group__server.html#ga02f2c6efa494dbbe1ee01e0d680f3bd6", null ],
    [ "pkrsrv_server_clients_new", "group__server.html#ga7aa718eac46e3c048e418a6e809d9753", null ],
    [ "pkrsrv_server_add_client", "group__server.html#ga1eeca32cd3a832c7f8561f92f3766ebd", null ],
    [ "pkrsrv_server_add_client__ts", "group__server.html#ga757b0e6895d9f4bb473d76b4b955e02d", null ],
    [ "pkrsrv_server_remove_client", "group__server.html#ga30ced3c227f282e9078c14472e9d9373", null ],
    [ "pkrsrv_server_remove_client__ts", "group__server.html#ga26d2eea8a99c882d3e4dd147e4faf25c", null ],
    [ "pkrsrv_server_client_new", "group__server.html#gab3559bbcb527bd1b62d9e468c185ee41", null ],
    [ "pkrsrv_server_client_free", "group__server.html#gae318f44fe4f22d06ff4593a0bae15e7d", null ],
    [ "pkrsrv_server_send_binary", "group__server.html#gad3933f38a0fe9774c41a77f3684fe538", null ],
    [ "pkrsrv_server_send_over_capacity", "group__server.html#gad555628bdb50b268f0224686347cfdc9", null ],
    [ "pkrsrv_server_send_pong", "group__server.html#ga830e4e8e563146d56a005560af25262a", null ],
    [ "pkrsrv_server_send_login_res", "group__server.html#ga815710d5d8bf4ffebabf0f772be90269", null ],
    [ "pkrsrv_server_send_signup_res", "group__server.html#ga37a8330aebe878d71b80b2fca739dbff", null ],
    [ "pkrsrv_server_send_auth_session_res", "group__server.html#gaadfd2427f57e77e23f366db823891c74", null ],
    [ "pkrsrv_server_send_account", "group__server.html#ga4fea3e33fdcf30c07227ba1b8d0dc264", null ],
    [ "pkrsrv_server_send_enter_res", "group__server.html#ga78dbf0f5cc70942184d322156d7d30c5", null ],
    [ "pkrsrv_server_send_leave_res", "group__server.html#ga63a2792f2fce8df620fc6428aae12bfa", null ],
    [ "pkrsrv_server_send_join_res", "group__server.html#gab763791bbc4f32d343b1a6ccb36dc8eb", null ],
    [ "pkrsrv_server_send_unjoin_res", "group__server.html#ga04d1d159dff1754ea29a1d019cbf6292", null ],
    [ "pkrsrv_server_send_poker_info", "group__server.html#ga4de30112837c89afcddecfd00b2b7867", null ],
    [ "pkrsrv_server_send_poker_info_player", "group__server.html#ga525b967207db87a8f4dc8973ece506ff", null ],
    [ "pkrsrv_server_send_poker_state", "group__server.html#gabacd5e4ccd9cc9afbcdf914605a93adf", null ],
    [ "pkrsrv_server_send_poker_state_player", "group__server.html#gaab21f3b2edc04d29b232bcc206370ad8", null ],
    [ "pkrsrv_server_send_poker_action_reflection", "group__server.html#gaed66fc77c58de7b0145ee6c0723f9a1d", null ],
    [ "pkrsrv_server_send_poker_end", "group__server.html#ga24e5df5a358dd6c0bce2f5a79de820c1", null ],
    [ "pkrsrv_server_send_poker_restarted", "group__server.html#ga9664d568e77217b41edd625b7d750732", null ],
    [ "pkrsrv_server_send_unjoined", "group__server.html#gaf1a8a82f1a15ebdf9c6112914ac45495", null ],
    [ "pkrsrv_server_send_tables", "group__server.html#ga14c9fda714fa2ab78050020d01781486", null ],
    [ "pkrsrv_server_send_sessions", "group__server.html#gabec4798fb518c1f24af8ade6d3b75967", null ],
    [ "pkrsrv_server_send_update_account_res", "group__server.html#gadc11c2ca37d76398c0b1d1db1eaff120", null ],
    [ "pkrsrv_server_send_server_info", "group__server.html#ga7bf7d08f1d9cb18de7a6b5b1eda2bc47", null ],
    [ "pkrsrv_server_send_json", "group__server.html#ga1dc251d2fc8177ad252273ccb36bedf1", null ],
    [ "pkrsrv_server::pkrsrv_server_new", "group__server.html#gad13cceb5dc89f70a8c77d7aeb0b473e6", null ],
    [ "pkrsrv_server::pkrsrv_server_start", "group__server.html#gac3ce3288aea013e27a7b505bf116c1c9", null ],
    [ "pkrsrv_server::pkrsrv_server_listen", "group__server.html#gab6d1fae9c8487d9a6afb4589ba2db6c0", null ],
    [ "pkrsrv_server::pkrsrv_server_free", "group__server.html#gafcdb70f08e80d4a37abc466c835fe95f", null ]
];