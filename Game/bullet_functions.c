/**
 * @brief 发射子弹
 * @param tank_id 发射坦克的ID
 */
void bullet_fire(uint8_t tank_id) {
    // 查找空闲的子弹槽
    int free_slot = -1;
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            free_slot = i;
            break;
        }
    }
    
    if (free_slot == -1) return; // 没有空闲槽位
    
    // 获取坦克炮口位置
    muzzle_pos_t muzzle = tank_get_muzzle_position(tank_id);
    float tank_angle = tank_get_angle(tank_id);
    
    // 初始化子弹
    bullets[free_slot].x = (float)muzzle.x;
    bullets[free_slot].y = (float)muzzle.y;
    
    // 计算子弹速度分量
    float angle_rad = tank_angle * M_PI / 180.0f;
    bullets[free_slot].vel_x = BULLET_SPEED * cos(angle_rad);
    bullets[free_slot].vel_y = BULLET_SPEED * sin(angle_rad);
    
    bullets[free_slot].color = (tank_id == 0) ? RED : BLUE;
    bullets[free_slot].active = 1;
    bullets[free_slot].spawn_time = HAL_GetTick();
    bullets[free_slot].owner_id = tank_id;
}

/**
 * @brief 更新所有子弹的位置和状态
 */
void bullet_update(void) {
    static uint32_t last_update = 0;
    uint32_t current_time = HAL_GetTick();
    
    if (last_update == 0) {
        last_update = current_time;
        return;
    }
    
    float delta_time = (current_time - last_update) / 1000.0f; // 转换为秒
    
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        
        // 检查子弹生存时间
        if (current_time - bullets[i].spawn_time > BULLET_LIFETIME) {
            bullets[i].active = 0;
            continue;
        }
        
        // 更新子弹位置
        bullets[i].x += bullets[i].vel_x * delta_time;
        bullets[i].y += bullets[i].vel_y * delta_time;
        
        // 检查边界碰撞（假设屏幕尺寸为160x128）
        if (bullets[i].x < 0 || bullets[i].x >= 160 || 
            bullets[i].y < 0 || bullets[i].y >= 128) {
            bullets[i].active = 0;
        }
    }
    
    last_update = current_time;
}

/**
 * @brief 绘制所有激活的子弹
 */
void bullet_draw_all(void) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        
        int x = (int)bullets[i].x;
        int y = (int)bullets[i].y;
        
        // 绘制子弹（简单的矩形）
        st7735_fill_rect(x - BULLET_SIZE/2, y - BULLET_SIZE/2, 
                        BULLET_SIZE, BULLET_SIZE, bullets[i].color);
    }
}

/**
 * @brief 清除屏幕上的子弹痕迹
 */
void bullet_clear_screen(void) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        
        int x = (int)bullets[i].x;
        int y = (int)bullets[i].y;
        
        // 用背景色清除子弹位置
        st7735_fill_rect(x - BULLET_SIZE/2, y - BULLET_SIZE/2, 
                        BULLET_SIZE, BULLET_SIZE, BLACK);
    }
}

/**
 * @brief 检查子弹与坦克的碰撞
 * @param bullet_idx 子弹索引
 * @param tank_id 坦克ID
 * @return 1表示碰撞，0表示未碰撞
 */
int bullet_check_collision(int bullet_idx, int tank_id) {
    if (!bullets[bullet_idx].active) return 0;
    if (bullets[bullet_idx].owner_id == tank_id) return 0; // 不与自己碰撞
    
    int tank_x, tank_y;
    tank_get_position(tank_id, &tank_x, &tank_y);
    
    int bullet_x = (int)bullets[bullet_idx].x;
    int bullet_y = (int)bullets[bullet_idx].y;
    
    // 简单的矩形碰撞检测
    int dx = abs(bullet_x - tank_x);
    int dy = abs(bullet_y - tank_y);
    
    return (dx < TANK_SIZE/2 + BULLET_SIZE && dy < TANK_SIZE/2 + BULLET_SIZE);
}

/**
 * @brief 处理所有碰撞检测
 */
void bullet_handle_collisions(void) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        
        // 检查与所有坦克的碰撞
        for (int tank_id = 0; tank_id < MAX_PLAYERS; tank_id++) {
            if (bullet_check_collision(i, tank_id)) {
                // 碰撞处理：销毁子弹，减少坦克生命值等
                bullets[i].active = 0;
                
                // 这里可以添加坦克受伤逻辑
                // 例如：tank_take_damage(tank_id, 1);
                
                break; // 一颗子弹只能击中一个目标
            }
        }
    }
}
