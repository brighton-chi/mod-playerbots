DELETE FROM ai_playerbot_texts WHERE name IN (
    'kalecgos_tank_sent_to_spectral_realm',
    'kalecgos_tank_should_enter_spectral_realm',
    'kalecgos_health_call_first_under',
    'kalecgos_health_call_hold',
    'kalecgos_health_call_together',
    'kalecgos_health_call_release',
    'kalecgos_end_hold',
    'kalecgos_sathrovarr_banished',
    'kiljaeden_designated_dragon_orb_user',
    'kiljaeden_no_designated_dragon_orb_user',
    'eredar_twins_alythess_tank_main_tank_paladin',
    'eredar_twins_alythess_tank_paladin_tank',
    'eredar_twins_alythess_tank_no_paladin'
);

DELETE FROM ai_playerbot_texts_chance WHERE name IN (
    'kalecgos_tank_sent_to_spectral_realm',
    'kalecgos_tank_should_enter_spectral_realm',
    'kalecgos_health_call_first_under',
    'kalecgos_health_call_hold',
    'kalecgos_health_call_together',
    'kalecgos_health_call_release',
    'kalecgos_end_hold',
    'kalecgos_sathrovarr_banished',
    'kiljaeden_designated_dragon_orb_user',
    'kiljaeden_no_designated_dragon_orb_user',
    'eredar_twins_alythess_tank_main_tank_paladin',
    'eredar_twins_alythess_tank_paladin_tank',
    'eredar_twins_alythess_tank_no_paladin'
);

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1948, 'kalecgos_tank_sent_to_spectral_realm', 'Tank %tank has been sent to the Spectral Realm. The active Kalecgos tank is %current.', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1949, 'kalecgos_tank_should_enter_spectral_realm', 'Tank %tank should enter the Spectral Realm. The active Kalecgos tank is %current.', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1950, 'kalecgos_health_call_first_under', '%boss is at 15% health!', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1951, 'kalecgos_health_call_hold', '%boss is at %health%. Stop attacking %low until %boss reaches 15%!', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1952, 'kalecgos_health_call_together', '%boss is at %health%. Keep attacking both together!', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1953, 'kalecgos_health_call_release', '%boss has reached 15%. Attack %low again!', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1954, 'kalecgos_end_hold', 'Kalecgos is at 5%. Stop attacking him until Sathrovarr is banished!', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1955, 'kalecgos_sathrovarr_banished', 'Sathrovarr is banished. Finish Kalecgos!', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1956, 'kiljaeden_designated_dragon_orb_user', '%bot is the first assistant bot and the designated dragon orb user. If you would like only players to control dragons, please remove assistant flags from all bots.', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1957, 'kiljaeden_no_designated_dragon_orb_user', 'No bot has an assistant flag, and therefore a player must control the dragons. If you would like a bot to control the dragons, please set the assistant flag for a bot.', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1958, 'eredar_twins_alythess_tank_main_tank_paladin', 'Alythess requires a Paladin tank. %bot is the main tank and a Paladin and is assigned to tank Alythess.', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1959, 'eredar_twins_alythess_tank_paladin_tank', 'Alythess requires a Paladin tank. The main tank is not a Paladin. %bot is the best-geared Paladin tank and is assigned to tank Alythess.', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1960, 'eredar_twins_alythess_tank_no_paladin', 'Alythess requires a Paladin tank. However, no Paladin tank is present. Therefore, the main tank, %bot, is assigned to tank Alythess.', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES
    ('kalecgos_tank_sent_to_spectral_realm', 100),
    ('kalecgos_tank_should_enter_spectral_realm', 100),
    ('kalecgos_health_call_first_under', 100),
    ('kalecgos_health_call_hold', 100),
    ('kalecgos_health_call_together', 100),
    ('kalecgos_health_call_release', 100),
    ('kalecgos_end_hold', 100),
    ('kalecgos_sathrovarr_banished', 100),
    ('kiljaeden_designated_dragon_orb_user', 100),
    ('kiljaeden_no_designated_dragon_orb_user', 100),
    ('eredar_twins_alythess_tank_main_tank_paladin', 100),
    ('eredar_twins_alythess_tank_paladin_tank', 100),
    ('eredar_twins_alythess_tank_no_paladin', 100);
