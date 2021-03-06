#include "ai_common.h"

#define angle	jump_time

void ai_heli(Entity *e) {
	if(e->dir) { // STOP changing direction!
		e->dir = 0;
		e->frame = 1;
	}
	
	switch(e->state) {
		case 0:		// stopped
		{
			Entity *b = entity_create(e->x + (16<<CSF), e->y - (9<<CSF) - (48<<CSF), 
					OBJ_HELICOPTER_BLADE, 0);
			b->linkedEntity = e;
			
			b = entity_create(e->x - (36<<CSF), e->y - (4<<CSF) - (48<<CSF), 
					OBJ_HELICOPTER_BLADE2, 0);
			b->linkedEntity = e;
			
			e->state++;
		}
		break;
		
		case 20:	// blades running
		break;
		
		case 30:	// blades running, spawn momorin
		{
			entity_create(e->x + (45<<CSF) - (56<<CSF), e->y + (34<<CSF) - (48<<CSF), 
					OBJ_MOMORIN, 0)->dir = 0;
			
			e->frame = 1;		// open hatch
			e->state++;
		}
		break;
		
		case 40:	// blades running, spawn momorin, santa, and chako (from credits)
		{
			entity_create(e->x + (47<<CSF) - (56<<CSF), e->y + (34<<CSF) - (48<<CSF), 
					OBJ_MOMORIN, 0)->dir = 0;
			entity_create(e->x + (34<<CSF) - (56<<CSF), e->y + (34<<CSF) - (48<<CSF), 
					OBJ_SANTA, 0)->dir = 0;
			entity_create(e->x + (21<<CSF) - (56<<CSF), e->y + (34<<CSF) - (48<<CSF), 
					OBJ_CHACO, 0)->dir = 0;
			
			e->frame = 1;		// open hatch
			e->state++;
		}
		break;
	}
}

void onspawn_heliblade1(Entity *e) {
	e->display_box = (bounding_box) { 56, 8, 56, 8 };
}

void onspawn_heliblade2(Entity *e) {
	e->display_box = (bounding_box) { 36, 8, 36, 8 };
}

void ai_heli_blade(Entity *e) {
	switch(e->state) {
		case 0:
		case 1:
		{
			if (e->linkedEntity && e->linkedEntity->state >= 20)
				e->state = 10;
		}
		break;
		
		case 10:
		{
			ANIMATE(e, 2, 0,1,2,1);
		}
		break;
	}
}

void ai_igor_balcony(Entity *e) {
	enum Frame { STAND1, STAND2, WALK1, WALK2, PUNCH1, PUNCH2, MOUTH1, 
				 JUMP, LAND, DEFEAT1, MOUTH2, DEFEAT2, DEFEAT3, DEFEAT4 };
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	uint8_t blockl = e->x_speed < 0 && collide_stage_leftwall(e);
	uint8_t blockr = e->x_speed > 0 && collide_stage_rightwall(e);
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);
	
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->grounded = FALSE;
			e->display_box.top += 4;
		} /* fallthrough */
		case 1:
		{
			ANIMATE(e, 20, STAND1,STAND2);
			
			if ((PLAYER_DIST_X(112<<CSF) && PLAYER_DIST_Y2(48<<CSF, 112<<CSF)) || e->damage_time) {
				e->state = 10;
			}
		}
		break;
		
		case 10:		// walking towards player
		{
			e->state = 11;
			e->frame = STAND1;
			e->animtime = 0;
			FACE_PLAYER(e);
			moveMeToFront = TRUE;
		} /* fallthrough */
		case 11:
		{
			ANIMATE(e, 8, WALK1,STAND1,WALK2,STAND1);
			MOVE_X(SPEED(0x200));
			
			if (blockr || blockl || PLAYER_DIST_X(64<<CSF)) {
				e->x_speed = 0;
				e->state = 20;
				e->timer = 0;
			}
		}
		break;
		
		case 20:	// prepare to jump...
		{
			e->frame = LAND;	// jump-prepare frame
			
			if (++e->timer > 10) {
				e->state = 21;
				e->y_speed = -SPEED(0x5ff);
				e->grounded = FALSE;
				MOVE_X(SPEED(0x200));
				sound_play(SND_IGOR_JUMP, 5);
				moveMeToFront = TRUE;
			}
		}
		break;
		
		case 21:	// jumping
		{
			e->frame = JUMP;	// in-air frame
			MOVE_X(SPEED(0x200));
			if (e->grounded) {
				camera_shake(20);
				e->x_speed = 0;
				
				e->state = 22;
				e->timer = 0;
				e->frame = LAND;
			}
		}
		break;
		
		case 22:	// landed
		{
			if (++e->timer > 30)
				e->state = 30;
		}
		break;
		
		case 30:	// mouth-blast attack
		{
			e->state = 31;
			e->timer = 0;
			FACE_PLAYER(e);
			moveMeToFront = TRUE;
		} /* fallthrough */
		case 31:
		{
			e->timer++;
			
			// flash mouth
			e->frame = MOUTH1;
			if (e->timer < TIME(50) && (e->timer & 4)) e->frame = MOUTH2;
			
			// fire shots
			if (e->timer > 30) {
				if ((e->timer & 7) == 1) {
					sound_play(SND_BLOCK_DESTROY, 5);
					Entity *shot = entity_create(e->x + (e->dir ? 0x800 : -0x800), 
												 e->y, OBJ_IGOR_SHOT, 0);
					shot->x_speed = SPEED(0x500) * (e->dir ? 1 : -1);
					shot->y_speed = SPEED(0x180) - (random() % SPEED(0x300));
				}
			}
			
			if (e->timer > 82) {
				FACE_PLAYER(e);
				e->state = 10;
			}
		}
		break;
	}
	
	e->x = e->x_next;
	e->y = e->y_next;
	
	if(!e->grounded) e->y_speed += SPEED(0x33);
	LIMIT_Y(SPEED(0x5ff));
}

void ai_block_spawner(Entity *e) {
	switch(e->state) {
		// wait till player leaves "safe zone" at start of Balcony
		// does nothing in Hell--you enter from the left.
		case 0:
		{
			if (player.x < block_to_sub(stageWidth - 6)) {
				e->state = 1;
				e->timer = 24;
			}
		}
		break;
		
		case 1:
		{
			if (--e->timer == 0) {
				//Entity *block;
				int x;
				
				// blocks tend to follow behind the player--this goes along
				// with the text that tells you to run so as not to get squashed.
				if (playerEquipment & EQUIP_BOOSTER20) {
					x = (player.x + block_to_sub(4));
					if (x < block_to_sub(26)) x = block_to_sub(26);
				} else {
					x = (player.x + block_to_sub(6));
					if (x < block_to_sub(23)) x = block_to_sub(23);
				}
				
				if (x > block_to_sub(stageWidth - 10))
					x = block_to_sub(stageWidth - 10);
				
				if (playerEquipment & EQUIP_BOOSTER20) {
					x += block_to_sub(-14 + (random() % 29));
				} else {
					x += block_to_sub(-11 + (random() % 23));
				}
				
				entity_create(x, (player.y - block_to_sub(14)), 
							OBJ_FALLING_BLOCK, (random() & 1) ? NPC_OPTION2 : 0);
									  
				e->timer = TIME(15) + (random() & 15);
			}
		}
		break;
	}
}

void ai_falling_block(Entity *e) {
	e->attack = (player.y > e->y) ? 10 : 0;
	
	switch(e->state) {
		case 0:
		{	
			if(e->eflags & NPC_OPTION2) { // large Hell or Balcony block
				e->eflags |= NPC_INVINCIBLE;
				e->state = 10;
			} else if(e->eflags & NPC_OPTION1) { // Misery-spawned block
				e->state = 1;
				e->timer = 0;
			} else { // small Hell or Balcony block
				e->eflags |= NPC_INVINCIBLE;
				e->state = 10; 
				e->hit_box = (bounding_box) { 8,8,8,8 };
				e->display_box = (bounding_box) { 8,8,8,8 };
				e->sheet++; // SHEET_BLOCKM always after SHEET_BLOCK
				e->vramindex = sheets[e->sheet].index;
				e->sprite[0].size = SPRITE_SIZE(2, 2);
			}
		}
		break;
		
		case 1:	// just spawned by Misery--pause a moment
		{
			if (++e->timer > 3) {
				e->eflags |= NPC_INVINCIBLE;
				e->state = 10;
			}
		}
		break;
		
		case 10:	// falling
		{	// allow to pass thru Hell/Balcony ceiling
			if (e->y > 128<<CSF) {
				e->state = 11;
			}
			e->y_speed += SPEED(0x40);
			LIMIT_Y(SPEED(0x5FF));
		}
		break;
		
		case 11:	// passed thru ceiling in Hell B2
		{
			e->y_speed += SPEED(0x40);
			LIMIT_Y(SPEED(0x5FF));
			
			if (blk(e->x, 0, e->y, (NPC_OPTION2 ? 8 : 20)) == 0x41) {
				e->y_speed = -SPEED(0x280);
				
				e->state = 20;
				SMOKE_AREA((e->x >> CSF) - 8, (e->y >> CSF) + (NPC_OPTION2 ? 8 : 16), 16, 1, 1);
				camera_shake(10);
			}
		}
		break;
		
		case 20:	// already bounced on ground, falling offscreen
		{
			e->attack = 0;
			e->y_speed += SPEED(0x40);
			LIMIT_Y(SPEED(0x5FF));
			
			if (e->y >= block_to_sub(stageHeight + 1)) {
				e->state = STATE_DELETE;
			}
		}
		break;
	}
	e->x += e->x_speed;
	e->y += e->y_speed;
}

// The Doctor in his red energy form.
// there is no "move" state, when he takes over Misery,
// the object is moved kind of unconventionally, using an <MNP.
void ai_doctor_ghost(Entity *e) {
	switch(e->state) {
		case 10:
		{
			e->state = 11;
			e->timer = 0;
		} /* fallthrough */
		case 11:
		{
			e->timer++;
			if((e->timer % TIME(5)) == 0) {
				Entity *r = entity_create(e->x, e->y+(128<<CSF), OBJ_RED_ENERGY, 0);
				r->angle = A_RIGHT;
				r->linkedEntity = e;
				if((e->timer % TIME(10)) == 0) r->timer = 200;
			}
			
			if (e->timer > TIME(150))
				e->state++;
		}
		break;
		
		case 20:
		{
			e->state = 21;
			e->timer = 0;
		} /* fallthrough */
		case 21:
		{
			if (++e->timer > TIME(250)) {
				entities_clear_by_type(OBJ_RED_ENERGY);
				e->state++;
			}
		}
		break;
	}
}

// red energy for doctor. In a completely different role,
// it's also used for the dripping blood from Ballos's final form.
void ai_red_energy(Entity *e) {
	switch(e->angle) {
		case A_UP:
		{
			e->y_speed -= SPEED(0x40);
			if (blk(e->x, 0, e->y, 0) == 0x41) e->state = STATE_DELETE;
		}
		break;
		
		case A_DOWN:
		{
			e->y_speed += SPEED(0x40);
			if (blk(e->x, 0, e->y, 0) == 0x41) e->state = STATE_DELETE;
			if (++e->timer > TIME(50)) e->state = STATE_DELETE;
			if (e->y_speed > SPEED(0x5ff)) e->y_speed = SPEED(0x5ff);
		}
		break;
		
		case A_RIGHT:
		{
			if (!e->linkedEntity || e->linkedEntity->state == STATE_DELETE) { 
				e->state = STATE_DELETE; 
				return; 
			}
			
			if(e->linkedEntity->state == 21 && ++e->timer > 200) {
				e->state = STATE_DELETE; 
				return;
			}
			
			if (e->state == 0) {
				e->state = 1;
				e->eflags |= NPC_IGNORESOLID;
				
				e->x_speed = -0x600 + (random() % 0x1200);
				e->y_speed = -0x600 + (random() % 0x1200);
				
				// accel speed
				//e->speed = (512 / random(16, 51));
				
				// x/y limit
				uint16_t limit = SPEED(0x80) + (random() & 0x7F);
				
				e->timer2 = limit << 1;			// x limit
				e->id = limit + (limit << 1);	// y limit (form elongated sphere)
			}
			
			int32_t tgtx = e->linkedEntity->x + (4<<CSF);
			if (e->x < tgtx) 		e->x_speed += SPEED(6);
			else if (e->x > tgtx)	e->x_speed -= SPEED(6);
			
			if (e->y < e->linkedEntity->y) 		e->y_speed += SPEED(6);
			else if (e->y > e->linkedEntity->y) e->y_speed -= SPEED(6);
			
			LIMIT_X(e->timer2);
			LIMIT_Y(e->id);
		}
	}
	
	e->x += e->x_speed;
	e->y += e->y_speed;
	
	e->frame = random() & 1;
}

void ai_mimiga_caged(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->x -= (1 << CSF);
			e->y -= (2 << CSF);
		} /* fallthrough */
		case 1:
		{
			e->frame = 0;
			RANDBLINK(e, 1, 200);
			
			if (e->frame == 0) FACE_PLAYER(e);
		}
		break;
		
		case 10:	// blush and spawn heart
		{
			e->state = 11;
			e->frame = 2;
			
			entity_create(e->x, e->y-(16<<CSF), OBJ_HEART, 0);
		} /* fallthrough */
		case 11:
		{
			FACE_PLAYER(e);
		}
		break;
	}
}
