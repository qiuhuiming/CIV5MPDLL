create table PromotionCollections (
    ID integer primary key autoincrement not null,
    Type text not null unique
);

create table PromotionCollections_Entries (
    CollectionType text not null references PromotionCollections(Type),
    PromotionType text not null references UnitPromotions(Type),
    PromotionIndex int not null default 0,
    
    TrigerMeleeAttack  boolean not null default 0,
    TrigerRangedAttack  boolean not null default 0,
    TrigerMeleeDefense boolean not null default 0,
    TrigerRangedDefense boolean not null default 0,
    TrigerHPFixed integer not null default 0,
    TrigerHPPercent integer not null default 0,
    TriggerLuaCheck boolean not null default 0,
    TriggerLuaHook boolean not null default 0
);
