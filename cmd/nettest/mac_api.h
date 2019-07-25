uint32_t mac_reg_read(MAC_ENGINE *p_eng, uint32_t addr);
void mac_reg_write(MAC_ENGINE *p_eng, uint32_t addr, uint32_t data);
void mac_set_delay(MAC_ENGINE *p_eng, int32_t rx_d, int32_t tx_d);
void mac_get_delay(MAC_ENGINE *p_eng, int32_t *p_rx_d, int32_t *p_tx_d);
void mac_set_driving_strength(MAC_ENGINE *p_eng, uint32_t strength);