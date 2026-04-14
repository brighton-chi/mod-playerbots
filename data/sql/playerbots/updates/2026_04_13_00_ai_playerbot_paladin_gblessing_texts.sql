DELETE FROM ai_playerbot_texts WHERE name IN ('paladin_gblessing_missing_reagents');
DELETE FROM ai_playerbot_texts_chance WHERE name IN ('paladin_gblessing_missing_reagents');

INSERT INTO ai_playerbot_texts
    (name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    ('paladin_gblessing_missing_reagents',
     'Missing reagents for %assigned_blessing. Using %fallback_blessing.',
     0, 0,
     '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES
    ('paladin_gblessing_missing_reagents', 100);