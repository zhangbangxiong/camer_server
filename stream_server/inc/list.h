/* This is list for wiizie server
 * file: list.h
 * author: zhangbangxiong
 * mail: zhangbangxiong@soooner.com
 * date: 2008-03-20 00:43
*/

#ifdef __cplusplus
# define EV_CPP(x) x
#else
# define EV_CPP(x)
#endif

EV_CPP(extern "C" {)

struct list_head 
{
    struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

static inline void __list_add(struct list_head *tnew,
                  struct list_head *prev,
                  struct list_head *next)
{
    next->prev = tnew;
    tnew->next = next;
    tnew->prev = prev;
    prev->next = tnew;
}

static inline void list_add_head(struct list_head *tnew, struct list_head *head)
{
    __list_add(tnew, head, head->next);
}

static inline void list_add_tail(struct list_head *tnew, struct list_head *head)
{
    __list_add(tnew, head->prev, head);
}

static inline void __list_del(struct list_head * prev, struct list_head * next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    entry->next = NULL;
    entry->prev = NULL;
}

static inline void list_replace(struct list_head *old,
                struct list_head *tnew)
{
    tnew->next = old->next;
    tnew->next->prev = tnew;
    tnew->prev = old->prev;
    tnew->prev->next = tnew;
}

static inline void list_replace_init(struct list_head *old,
                    struct list_head *tnew)
{
    list_replace(old, tnew);
    INIT_LIST_HEAD(old);
}
static inline void list_del_init(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    INIT_LIST_HEAD(entry);
}

static inline void list_move(struct list_head *list, struct list_head *head)
{
    __list_del(list->prev, list->next);
    list_add_head(list, head);
}

static inline void list_move_tail(struct list_head *list,
                  struct list_head *head)
{
    __list_del(list->prev, list->next);
    list_add_tail(list, head);
}
static inline int list_is_last(const struct list_head *list,
                const struct list_head *head)
{
    return list->next == head;
}

static inline int list_empty(const struct list_head *head)
{
    return head->next == head;
}

static inline int list_next_empty(const struct list_head *head)
{
    if (head->next == head)
        return 1;

    return head->next->next == head;
}

#define list_offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define container_of(ptr, type, member) \
    ({ const typeof( ((type *)0)->member ) *__mptr = (ptr); (type *)( (char *)__mptr - list_offsetof(type,member) );})

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)

#define list_second_entry(ptr, type, member) \
    list_entry((ptr)->next->next, type, member)

#define list_last_entry(ptr, type, member) \
    list_entry((ptr)->prev, type, member)

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); \
        pos = pos->next)

#define __list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_prev(pos, head) \
    for (pos = (head)->prev; pos != (head); \
            pos = pos->prev)

#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
        pos = n, n = pos->next)

#define list_for_each_prev_safe(pos, n, head) \
    for (pos = (head)->prev, n = pos->prev; \
         pos != (head); pos = n, n = pos->prev)

#define list_for_each_entry(pos, head, member)              \
    for (pos = list_entry((head)->next, typeof(*pos), member);  \
         &pos->member != (head); pos = list_entry(pos->member.next, typeof(*pos), member))

#define list_for_each_prev_entry(pos, head, member)              \
    for (pos = list_entry((head)->prev, typeof(*pos), member);  \
         &pos->member != (head); pos = list_entry(pos->member.prev, typeof(*pos), member))

#define list_for_each_entry_safe(pos, n, head, member)          \
    for (pos = list_entry((head)->next, typeof(*pos), member),  \
        n = list_entry(pos->member.next, typeof(*pos), member); \
         &pos->member != (head);                    \
         pos = n, n = list_entry(n->member.next, typeof(*n), member))

#define list_for_each_in_portion(pos, head, tail) \
    for (pos = (head); pos != (tail)->next; pos = pos->next)

#define list_entry_count(head, n) \
    ({ n = 0; struct list_head *pos = NULL; for (pos = (head)->next; pos != (head); n++, pos = pos->next); })


EV_CPP(})

