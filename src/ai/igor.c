#include "ai_common.h"

#define fireatk curly_target_x

enum Frame { STAND1, STAND2, WALK1, WALK2, PUNCH1, PUNCH2, MOUTH1, 
			 JUMP, LAND, DEFEAT1, MOUTH2, DEFEAT2, DEFEAT3, DEFEAT4 };

void onspawn_igor(Entity *e) {
	fireatk = 0;
	e->attack = 0;
	e->hit_box.bottom += 4;
	e->hit_box.top -= 4;
	if(e->type == 0x59) e->frame = 7;
}

void ai_igor(Entity *e) {
	enum {
		STATE_STAND = 0,
		STATE_WALK = 10,
		STATE_JUMPING = 20,
		STATE_LANDED = 30,
		STATE_PUNCH = 40,
		STATE_MOUTH_BLAST = 50,
	};
	switch(e->state) {
		case STATE_STAND:
		{
			e->alwaysActive = TRUE;
			e->attack = 0;
			e->timer = 0;
			e->state++;
		} /* fallthrough */
		case STATE_STAND+1:
		{
			ANIMATE(e, 16, STAND1, STAND2);
			if(++e->timer > TIME(50)) e->state = STATE_WALK;
		}
		break;
		case STATE_WALK:
		{
			FACE_PLAYER(e);
			e->timer = 0;
			// when health is less than halfway, then use
			// the mouth blast attack every third time.
			if(++fireatk >= 3 && e->health <= npc_hp(e->type) / 2) {
				fireatk = -1;
				e->dir ^= 1;	// walk away from player
			}
			e->x_speed = e->dir ? SPEED(1<<CSF) : -SPEED(1<<CSF);
			e->state++;
			moveMeToFront = TRUE;
		} /* fallthrough */
		case STATE_WALK+1:
		{
			ANIMATE(e, 12, WALK1,STAND1,WALK2,STAND1);
			if(fireatk == -1) {	// begin mouth-blast attack
				if(++e->timer > TIME(20)) e->state = STATE_MOUTH_BLAST;
			} else {
				// if we don't reach him after a while, do a jump
				if(++e->timer > TIME(50)) {
					e->state = STATE_JUMPING;
				} else if(e->dir) {
					if(e->x >= player.x - pixel_to_sub(22)) e->state = STATE_PUNCH;
				} else {
					if(e->x <= player.x + pixel_to_sub(22)) e->state = STATE_PUNCH;
				}
			}
		}
		break;
		case STATE_PUNCH:
		{
			e->frame = PUNCH1;
			e->x_speed = 0;
			e->timer = 0;
			e->state++;
			moveMeToFront = TRUE;
		} /* fallthrough */
		case STATE_PUNCH+1:
		{
			if(++e->timer > TIME(16)) e->state++;
		}
		break;
		case STATE_PUNCH+2:
		{
			sound_play(SND_EXPL_SMALL, 5);
			// sprite appears identical, but has a wider bounding box.
			if(e->dir) {
				e->hit_box.right += 10;
			} else {
				e->hit_box.left += 10;
			}
			e->frame = PUNCH2;
			e->attack = 5;
			e->timer = 0;
			e->state++;
		} /* fallthrough */
		case STATE_PUNCH+3:
		{
			if(++e->timer > TIME(14)) {
				// return to normal-size bounding box
				if(e->dir) {
					e->hit_box.right -= 10;
				} else {
					e->hit_box.left -= 10;
				}
				e->state = STATE_STAND;
			}
		}
		break;
		case STATE_JUMPING:
		{
			e->frame = JUMP;
			e->y_speed = -SPEED(2<<CSF);
			e->grounded = FALSE;
			e->attack = 2;
			e->x_speed *= 2;
			e->x_speed /= 3;
			e->x_mark = e->x_speed; // Remember x speed
			e->timer = 0;
			e->state++;
			moveMeToFront = TRUE;
		} /* fallthrough */
		case STATE_JUMPING+1:
		{
			if(e->grounded) {
				sound_play(SND_ENEMY_JUMP, 5);
				effect_create_smoke(sub_to_pixel(e->x), sub_to_pixel(e->y) + e->hit_box.bottom);
				e->state = STATE_LANDED;
			} else {
				// Don't stop momentum if we hit the step on the left
				if(e->x_speed == 0) e->x_speed = e->x_mark;
			}
		}
		break;
		case STATE_LANDED:
		{
			e->frame = LAND;
			e->x_speed = 0;
			e->timer = 0;
			e->state++;
			moveMeToFront = TRUE;
		} /* fallthrough */
		case STATE_LANDED+1:
		{
			if(++e->timer > TIME(12)) e->state = STATE_STAND;
		}
		break;
		case STATE_MOUTH_BLAST:
		{
			e->frame = MOUTH1;
			FACE_PLAYER(e);
			e->x_speed = 0;
			e->timer = 0;
			e->state++;
			moveMeToFront = TRUE;
		} /* fallthrough */
		case STATE_MOUTH_BLAST+1:
		{
			e->frame = (++e->timer > TIME(50) && (e->timer & 4)) ? MOUTH2 : MOUTH1;
			// fire shots
			if(e->timer > TIME(100)) {
				if((e->timer % 8) == 1) {
					sound_play(SND_BLOCK_DESTROY, 5);
					Entity *shot = entity_create(e->x + (e->dir ? 0x800 : -0x800), 
							e->y, OBJ_IGOR_SHOT, 0);
					shot->x_speed = SPEED(0x500) * (e->dir ? 1 : -1);
					shot->y_speed = SPEED(0x180) - (random() % SPEED(0x300));
				}
				// fires 6 shots
				if(e->timer > TIME(135)) e->state = STATE_STAND;
			}
		}
		break;
	}
	if(!e->grounded) e->y_speed += SPEED(0x20);
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

void ondeath_igor(Entity *e) {
	e->attack = 0;
	e->eflags &= ~(NPC_SHOOTABLE|NPC_SHOWDAMAGE);
	e->x_speed = 0;
	tsc_call_event(e->event); // Boss defeated event
}

void ai_igorscene(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);
	switch(e->state) {
		case 0:
		{
			// Push Sue in front if she exists
			Entity *find = entityList;
			while(find && find->type != OBJ_SUE) find = find->next;
			if(find) {
				entity_deactivate(find);
				entity_reactivate(find);
			}
			e->x_speed = 0;
			e->timer = 0;
			e->state++;
		} /* fallthrough */
		case 1:
		{
			ANIMATE(e, 16, STAND1,STAND2);
		}
		break;
		case 2:
		{
			MOVE_X(SPEED(0x200));
		} /* fallthrough */
		case 3:
		{
			ANIMATE(e, 12, WALK1,STAND1,WALK2,STAND1);
		}
		break;
		case 4:
		{
			e->x_speed = 0;
			e->frame = PUNCH1;
			e->timer = 20;
			e->state++;
		} /* fallthrough */
		case 5:
		{
			e->timer--;
			if(!e->timer) {
				e->frame = PUNCH2;
				e->timer = 20;
				e->state = 6;
			}
		}
		break;
		case 6:
		{
			e->timer--;
			if(!e->timer) e->state = 0;
		}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
	if(!e->grounded) e->y_speed += SPEED(0x40);
}

void ai_igordead(Entity *e) {
	switch(e->state) {
		case 0:
		FACE_PLAYER(e);
		sound_play(SND_BIG_CRASH, 5);
		//SmokeBoomUp(e);
		e->x_speed = 0;
		e->timer = 0;
		e->frame = DEFEAT1;
		e->state = 1;
		break;
		case 1:
		// Puffs of smoke
		if((++e->timer % 10) == 1) {
			effect_create_smoke(sub_to_pixel(e->x) - 24 + (random() % 48), 
				sub_to_pixel(e->y) - 32 + (random() % 64));
		}
		// Shake
		if((e->timer & 3) == 1) {
			e->display_box.left -= 1;
		} else if((e->timer & 3) == 3) {
			e->display_box.left += 1;
		}
		if(e->timer > 100) {
			e->timer = 0;
			e->state = 2;
		}
		break;
		case 2:
		// Slower smoke puff
		if((++e->timer & 15) == 0) {
			effect_create_smoke(sub_to_pixel(e->x) - 24 + (random() % 48), 
				sub_to_pixel(e->y) - 32 + (random() % 64));
		}
		// alternate between big and small sprites
		// (frenzied/not-frenzied forms)
		if((e->timer & 3) == 1) {
			e->frame = DEFEAT2;
		} else if((e->timer & 3) == 3) {
			e->frame = DEFEAT1;
		}
		if(e->timer > 160) {
			e->frame = DEFEAT2;
			e->state = 3;
			e->timer = 0;
		}
		break;
		case 3:
		if(++e->timer > 60) {
			e->frame++;
			e->timer = 0;
			if(e->frame > DEFEAT4) {
				e->hidden = TRUE;
				e->state = 4;
			}
		}
		//effect_create_smoke(e->x >> CSF, (e->y >> CSF) + 20);
		break;
		case 4: break;
	}
}
