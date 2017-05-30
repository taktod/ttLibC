/*
 * @file   linkedListUtil.c
 * @brief  linkedList data structure support.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/11/26
 * linkedlistはメモリーが許す限りいくらでも配列を増やすことが可能だが、CPU的にはメモリーアクセスしにくい。
 * データが断片的にあちこちにある状況になるため。
 */

#include "linkedListUtil.h"
#include <stdio.h>
#include "../allocator.h"
#include "../_log.h"
#include <string.h>

/**
 * linkedListのnode
 */
typedef struct ttLibC_Util_LinkedListNode_t{
	// 宣言の前方参照ってできたっけ？
	struct ttLibC_Util_LinkedListNode_t *next; // とりあえずvoidポインターにしておく・・・なんか妙な気分だ・・・
	// とりあえず降順のアクセスはできないようにしておく。必要があればつくればいいかな。
//	struct ttLibC_Util_LinkedListNode_t *prev;
	// 保持データはitemとしておくけど・・・メモリー解放が面倒なので、unionの方がいいかな・・・うーん。
	void *item;
	size_t item_size;
	// itemのデータがrefのみの場合はtrueをいれておく(解放時にfreeしない) falseがはいっている場合は、実体コピーなのでttLibC_freeで解放しておく。
	bool is_non_copy;
} ttLibC_Util_LinkedListNode;

typedef ttLibC_Util_LinkedListNode ttLibC_LinkedListNode;

// ここにnode用の関数を定義しておく必要がありそう。
static ttLibC_LinkedListNode *LinkedListNode_make(void *item, size_t item_size, bool is_non_copy) {
	LOG_PRINT("nodeの作成を実施する。");
	ttLibC_LinkedListNode *node = ttLibC_malloc(sizeof(ttLibC_LinkedListNode));
	if(node == NULL) {
		return NULL;
	}
	// データの保持まわりを作成しておく。
	node->next = NULL;
	node->is_non_copy = is_non_copy;
	node->item_size = item_size;
	if(is_non_copy) {
		// データをコピーしない場合はポインタを保持しておけばOK
		node->item = item;
	}
	else {
		// データをコピーする場合はコピーする。
		node->item = ttLibC_malloc(item_size);
		memcpy(node->item, item, item_size);
	}
	return node;
}

static void LinkedListNode_close(ttLibC_LinkedListNode **node) {
	LOG_PRINT("nodeの解放を実施する。");
	ttLibC_LinkedListNode *target = *node;
	if(target == NULL) {
		return;
	}
	if(!target->is_non_copy) {
		// 実体を持つデータの場合は解放しなければならない。
		ttLibC_free(target->item);
	}
	target->next = NULL; // このnull埋めはいらないかもしれませんね。
	ttLibC_free(target);
	*node = NULL;
}

/*
 * detail definition of linked list.
 */
typedef struct {
	ttLibC_LinkedList inherit_super;
	ttLibC_LinkedListNode *first; // 一番はじめのnodeへのアクセス
	// lastの処理はいれておくと、楽に処理できるけど、考えるのが面倒になるので、一旦放置しておく。
//	ttLibC_LinkedListNode *last; // 終端にデータを追加・・・を結構やりそうなので、ポインタを保持しておく。
	size_t size; // 保持要素サイズ
} ttLibC_Util_LinkedList_;

typedef ttLibC_Util_LinkedList_ ttLibC_LinkedList_;

ttLibC_LinkedList *ttLibC_LinkedList_make() {
	LOG_PRINT("make is called.");
	ttLibC_LinkedList_ *linkedList = ttLibC_malloc(sizeof(ttLibC_LinkedList_));
	if(linkedList == NULL) {
		return NULL;
	}
	linkedList->first = NULL;
//	linkedList->last = NULL;
	linkedList->size = 0;
	linkedList->inherit_super.size = linkedList->size;
	return (ttLibC_LinkedList *)linkedList;
}

// 先頭にデータを追加する。
bool ttLibC_LinkedList_addFirst(
		ttLibC_LinkedList *list,
		void *add_item,
		size_t add_item_size,
		bool is_non_copy) {
	LOG_PRINT("add first is called.");
	ttLibC_LinkedList_ *linkedList_ = (ttLibC_LinkedList_ *)list;
	ttLibC_LinkedListNode *node = LinkedListNode_make(add_item, add_item_size, is_non_copy);
	if(node == NULL) {
		LOG_PRINT("nodeが作成されていません。");
		return false;
	}
	++ linkedList_->size;
	linkedList_->inherit_super.size = linkedList_->size;
	// 初めのところに追加すればそれでよい。
	// 現状の先頭nodeを保持
	ttLibC_LinkedListNode *next_node = linkedList_->first;
	linkedList_->first = node;
	node->next = next_node;
	// これでOKのはず。
	return false;
}

// 終端にデータを追加する
bool ttLibC_LinkedList_addLast(
		ttLibC_LinkedList *list,
		void *add_item,
		size_t add_item_size,
		bool is_non_copy) {
	LOG_PRINT("addLast is called.");
	ttLibC_LinkedList_ *linkedList_ = (ttLibC_LinkedList_ *)list;
	// 追加アイテムについて、nodeオブジェクトをつくる。
	// linkを更新する。
	// is non copyがtrueの場合は、オブジェクトをコピーしないので、そのままnodeに格納
	// is non copyがfalseの場合は、add_item_size分のメモリーを準備する必要がある。
	// nodeをつくる
	ttLibC_LinkedListNode *node = LinkedListNode_make(add_item, add_item_size, is_non_copy);
	if(node == NULL) {
		// nodeが生成されない場合は動作エラー
		LOG_PRINT("nodeが生成されていません。");
		return false;
	}
	++ linkedList_->size;
	linkedList_->inherit_super.size = linkedList_->size;
	// nodeができたので、ひも付きつくっておく。
	if(linkedList_->first == NULL) {
		LOG_PRINT("初めての追加データの場合");
		linkedList_->first = node;
//		linkedList_->last  = node;
		return true;
	}
	LOG_PRINT("初めてじゃない場合の追加の場合 未作成");
	ttLibC_LinkedListNode *target_node = linkedList_->first;
	while(target_node->next != NULL) {
		// 次のデータが見つからなければ一番最後である
		target_node = target_node->next;
	}
	target_node->next = node;
	return true;
}

/*
// after
bool ttLibC_LinkedList_addAfter(
		ttLibC_LinkedList *list,
		void *item,
		void *add_item,
		size_t add_item_size,
		bool is_non_copy) {
	ttLibC_LinkedList_ *linkedList_ = (ttLibC_LinkedList_ *)list;
	return false;
}

bool ttLibC_LinkedList_addBefore(
		ttLibC_LinkedList *list,
		void *item,
		void *add_item,
		size_t add_item_size,
		bool is_non_copy) {
	ttLibC_LinkedList_ *linkedList_ = (ttLibC_LinkedList_ *)list;
	return false;
}

// ややこしいので、この実装は後回しにしておく
*/
bool ttLibC_LinkedList_remove(
		ttLibC_LinkedList *list,
		void *remove_item) {
	ttLibC_LinkedList_ *linkedList_ = (ttLibC_LinkedList_ *)list;
	/*
	 * このremove関数は、内部でデータコピーをつくったときに、オリジナルデータとポイントするアドレスが違うため、削除できないという不具合がでる可能性がある。
	 * よって、forEachのループ上でのみ削除可能であるとする。(データのアドレスが一致するので、綺麗に削除可能になるはず。)
	 * 非推奨となるけど、forEachで応答されるデータを保持しつつ・・・というのは可能
	 */
	// listの中身を検索して、消すべきnodeを見つける。
	ttLibC_LinkedListNode *node = linkedList_->first;
	if(node == NULL) {
		return false; // データがない
	}
	// データがあるはずなので、データを確認する
	if(node->item == remove_item) {
		// pointerなので、アドレスが同じなら、消すべき対象であると考える。
		// このnodeを消す必要がある。
		-- linkedList_->size;
		linkedList_->inherit_super.size = linkedList_->size;
		linkedList_->first = node->next; // 次のデータをfirstにいれておく
		LinkedListNode_close(&node); // 今回のデータは消しておく。
		// おしまい。
		return true;
	}
	while(node->next != NULL) {
		// NULLじゃなかったら中身を確認する。
		if(node->next->item == remove_item) {
			// ループの中でこれを実行するとループがこわれるのか・・・
			LOG_PRINT("削除すべきデータをみつけた。");
			ttLibC_LinkedListNode *remove_node = node->next;
			-- linkedList_->size;
			linkedList_->inherit_super.size = linkedList_->size;
			node->next = node->next->next;
			LinkedListNode_close(&remove_node);
			return true;
		}
		else {
			LOG_PRINT("削除すべきデータではなかった。");
		}
	}
	// 削除すべきデータがなかった。
	return false;
}

// こっちの実装は簡単なので、すぐに実装しておく
bool ttLibC_LinkedList_removeAll(ttLibC_LinkedList *list) {
	ttLibC_LinkedList_ *linkedList_ = (ttLibC_LinkedList_ *)list;
	// listの中にあるnodeをすべて削除する。
	// こっちは普通に前から順に削除していけばOK
	ttLibC_LinkedListNode *node = linkedList_->first;
	while(node != NULL) {
		ttLibC_LinkedListNode *next_node = node->next;
		LinkedListNode_close(&node); // 現在のnodeをクリアする。
		node = next_node; // 次のnodeを処理する。
	}
	linkedList_->first = NULL;
	linkedList_->size = 0;
	linkedList_->inherit_super.size = linkedList_->size;
	return true;
}

// 関数で見つけたものを応答する形にしたい。
bool ttLibC_LinkedList_forEach(
		ttLibC_LinkedList *list,
		ttLibC_LinkedListRefFunc callback,
		void *ptr) {
	LOG_PRINT("forEach is called.");
	ttLibC_LinkedList_ *linkedList_ = (ttLibC_LinkedList_ *)list;
	ttLibC_LinkedListNode *node = linkedList_->first;
	while(node != NULL) {
		// callbackを実施する前に次のnodeをしっておく必要がありそう。
		ttLibC_LinkedListNode *next_node = node->next;
		if(!callback(ptr, node->item, node->item_size)) {
			// falseが帰ってきたら処理を抜ける。
			return false;
		}
		node = next_node;
	}
	return true;
}

void ttLibC_LinkedList_close(ttLibC_LinkedList **list) {
	LOG_PRINT("close is called.");
	ttLibC_LinkedList_ *target = (ttLibC_LinkedList_ *)*list;
	if(target == NULL) {
		return;
	}
	// 保持データをすべて解放する。
	ttLibC_LinkedList_removeAll((ttLibC_LinkedList *)target);
	ttLibC_free(target);
	*list = NULL;
}
