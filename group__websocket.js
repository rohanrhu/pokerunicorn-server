var group__websocket =
[
    [ "pkrsrv_websocket_header_t", "group__websocket.html#structpkrsrv__websocket__header__t", [
      [ "header0_16", "group__websocket.html#a71b15a3fc0ee18f860a5a955cd9f0fac", null ],
      [ "fin", "group__websocket.html#afc8613443c91cd250df91ffb8ee68071", null ],
      [ "plen", "group__websocket.html#a376a2507c4b10440b532eb048dd4558a", null ],
      [ "plen16", "group__websocket.html#a75768d20aac4def24165fb11db1b442f", null ],
      [ "plen64", "group__websocket.html#af48886e14c8d2841dbd5554330216089", null ],
      [ "opcode", "group__websocket.html#a7031ce2c9bbcde4fb870ba0dbe7943a2", null ],
      [ "is_masked", "group__websocket.html#a8f8f813a2b0c620f2648beb12c1e009a", null ],
      [ "mkey", "group__websocket.html#a595c1752c309d49bcd5db888eb846d0a", null ],
      [ "mask_i", "group__websocket.html#a042dfb7d35707314467453fce94348bb", null ]
    ] ],
    [ "pkrsrv_websocket_payload_t", "group__websocket.html#structpkrsrv__websocket__payload__t", [
      [ "length", "group__websocket.html#a313cdcb4dafcbc823bd16718074cf1c2", null ],
      [ "data", "group__websocket.html#a53c07f3af08e7f9b6a7c1ba2983aa222", null ]
    ] ],
    [ "pkrsrv_websocket_t", "group__websocket.html#structpkrsrv__websocket__t", [
      [ "read", "group__websocket.html#afa634620a60011bbd9b9b681989f38bf", null ],
      [ "write", "group__websocket.html#af0f708df93719b55b808c9557d8419a6", null ],
      [ "current_header", "group__websocket.html#a202baceaca86aedf3652ae934c9159db", null ],
      [ "is_fragmented", "group__websocket.html#a0327ec75af74837e09c8e494246d5697", null ],
      [ "buffer", "group__websocket.html#a0dfe47ad03bd4ae29902a0e2ec0c8b67", null ],
      [ "buffer_length", "group__websocket.html#ae12405785708974cac0093a81f62abad", null ],
      [ "buffer_readed", "group__websocket.html#afc9466c3ef67653a0b6986a75bec226c", null ],
      [ "write_mutex", "group__websocket.html#ab09dfb1c8b5b1965908ab9270b5d8019", null ]
    ] ],
    [ "PKRSRV_WEBSOCKET_EXPECTED_HTTP_HEADER", "group__websocket.html#gadfd1f28512035ce667531d7f932646ac", null ],
    [ "pkrsrv_websocket_packet_frame_header", "group__websocket.html#ga8184771ccefcd64a0d065a0a9499d5bf", null ],
    [ "PKRSRV_WEBSOCKET_GUID", "group__websocket.html#ga284a0a52e12c99a91c956dc313651f3a", null ],
    [ "HTTP_HEADER_BUFF_SIZE", "group__websocket.html#ga8c7943b13bdf7def5b72fdb7d5b44cb4", null ],
    [ "HTTP_PROP_BUFF_SIZE", "group__websocket.html#gaf9148aeceb825bfa686aa47134cd504d", null ],
    [ "HTTP_VAL_BUFF_SIZE", "group__websocket.html#ga475ad95a9e3279de972be0b321f57dba", null ],
    [ "pkrsrv_websocket_packet_frame_len8_t", "group__websocket.html#ga88823f8ee6b67d324068d14dbc647f27", null ],
    [ "pkrsrv_websocket_packet_frame_len16_t", "group__websocket.html#ga6dd35c76b8d0325cc2664533f23c3400", null ],
    [ "pkrsrv_websocket_packet_frame_len64_t", "group__websocket.html#ga9c069adf2ab7f85f9f42415e2dcbe6ff", null ],
    [ "PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE", "group__websocket.html#ga961af1df504746a586aa03cc74f63169", [
      [ "PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_METHOD", "group__websocket.html#gga961af1df504746a586aa03cc74f63169ad3fc74fa0898711100b86fe148f4614e", null ],
      [ "PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_PROP", "group__websocket.html#gga961af1df504746a586aa03cc74f63169a063d231070803dc65283c80fe3235692", null ],
      [ "PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_SPACE", "group__websocket.html#gga961af1df504746a586aa03cc74f63169afcd4d0bd8cfc9459fb78e936bfc2900f", null ],
      [ "PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_VAL", "group__websocket.html#gga961af1df504746a586aa03cc74f63169a025155cbdf1974cb294186aaba820f7b", null ],
      [ "PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_CR", "group__websocket.html#gga961af1df504746a586aa03cc74f63169a3c7395c53b6be48fc81bef315334d934", null ],
      [ "PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_END", "group__websocket.html#gga961af1df504746a586aa03cc74f63169a53181f0bf034a9ec81b45598b237a8a9", null ]
    ] ],
    [ "PKRSRV_WEBSOCKET_RESPONSE", "group__websocket.html#ga88f04cf22ce035730230c799e428fb13", [
      [ "PKRSRV_WEBSOCKET_RESPONSE_INSTANCE_PORT", "group__websocket.html#gga88f04cf22ce035730230c799e428fb13aebdad065c184399c5c4429e4c77aadcc", null ]
    ] ],
    [ "__attribute__", "group__websocket.html#ga3d7e9860438e08f764f33a3f79f6e843", null ],
    [ "pkrsrv_websocket_generate_ws_accept_key", "group__websocket.html#ga82fffe2c5a9e56550820ae946db50567", null ],
    [ "pkrsrv_websocket_init", "group__websocket.html#gab4e528c7e9909068a2350f00234ea28a", null ],
    [ "pkrsrv_websocket_read_http_header", "group__websocket.html#gadedbfb5391a52ce3ccc858679b2967a1", null ],
    [ "pkrsrv_websocket_read_header", "group__websocket.html#gad5e648a6a8b796bbbf10fbf681e28f56", null ],
    [ "pkrsrv_websocket_read_payload", "group__websocket.html#ga634de1c7ea01451220b6a7d8c42ea822", null ],
    [ "pkrsrv_websocket_send_header", "group__websocket.html#gad52de879d5fd46ff516b1966d9ad977b", null ],
    [ "pkrsrv_websocket_fragmented_begin", "group__websocket.html#ga96d620dbf4234161d2e16e196830a7dd", null ],
    [ "pkrsrv_websocket_fragmented_put", "group__websocket.html#gada9f02c16bb7b7e8ee153fa35b3bcd33", null ]
];