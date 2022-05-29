#include "huffstuff.h"

/*takes frequency so it doesn't depend on a file pointer
input which isn't used by hencode or hdecode,
returns the ordered node list with their codes */
huffnode **allhuff(int freqlist[]) {
  huffnode **nodelist;
  int size = 0;
  /* 2 so easy to distinguish errors */
  char codes[ASCIIVALS] = {'2'};
  int i;
  huffnode *tempnode;
  huffnode *huffll;
  nodelist = calloc(ASCIIVALS, sizeof(huffnode *));
  if (nodelist == NULL) {
    perror("Nodelist calloc failed");
    exit(3);
  }
  /* creates list of huffnodes */
  for (i = 0; i < ASCIIVALS; i++) {
    if (freqlist[i] != 0) {
      tempnode = malloc(sizeof(huffnode));
      if (tempnode == NULL) {
	perror("Tempnode malloc failed");
	exit(3);
      }
      tempnode->freq = freqlist[i];
      tempnode->char_val = i;
      tempnode->left = NULL;
      tempnode->right = NULL;
      nodelist[size] = tempnode;
      size++;
    }
  }
  /* empty case */
  if (nodelist[0] == NULL) {
    fprintf(stderr, "empty input");
    return 0;
  }
  /* resize nodelist down to size */
  nodelist = realloc(nodelist, size*sizeof(huffnode *));
  if (nodelist == NULL) {
    perror("Nodelist realloc failed");
    exit(3);
  }
  /* sort nodelist */
  qsort(nodelist, size, sizeof(huffnode *), comp);
  /* convert list into a linked list */
  for (i = 0; i < size - 1; i++) {
    nodelist[i]->next = nodelist[i + 1];
  }
  nodelist[size - 1]->next = NULL;
  /* build tree */
  huffll = build_tree(nodelist, size);
  /* make codes */
  if (huffll != NULL) {
    create_codes(huffll, codes, 0);
  }
  /* re-sort nodelist by ascii value */
  qsort(nodelist, size, sizeof(huffnode *), comp2);
  /*need a way to pass size back */
  nodelist[0]->size = size;

  return nodelist;
}

/* takes in a huffnode and a list and inserts the node 
to the correct spot in the list by increasing frequency,
tie-breaking by the newest node first */
huffnode *insert(huffnode *list, huffnode *item) {
  huffnode *cur;
  huffnode *prev;
  /* check if last item */
  if (list == NULL) {
    return item;
  }
  cur = list->next;
  prev = list;
  /* check for insert at 0 */
  if (prev->freq >= item->freq) {
    item->next = prev;
    return item;
  }
  /* general insert */
  while (cur != NULL) {
    if (cur->freq >= item->freq) {
      prev->next = item;
      item->next = cur;
      return list;
    }
    prev = cur;
    cur = cur->next;
  }
  /* insert at end */
  prev->next = item;
  item->next = NULL;
  return list;
}

/* takes two huffnodes and combines them into a parent node
with the input nodes as children and freqency as the sum
of the children nodes' frequencies*/
huffnode *combine(huffnode *a, huffnode *b) {
  unsigned char char_val;
  huffnode *node = malloc(sizeof(huffnode));
  if (node == NULL) {
    perror("Combine malloc failed");
    exit(3);
  }
  if (a->char_val < b->char_val) {
    char_val = a->char_val;
  } else {
    char_val = b->char_val;
  }
  /* include char val for debugging purposes */
  node->char_val = char_val;
  node->freq = a->freq + b->freq;
  node->left = a;
  node->right = b;

  return node;
}

/* compares two huffnodes by frequency then by ascii val,
returns the lesser for both cases */
int comp(const void *a, const void *b) {
  huffnode **node1 = (huffnode **)a;
  huffnode **node2 = (huffnode **)b;

  if ((*node1)->freq != (*node2)->freq) {
    return ((*node1)->freq - (*node2)->freq);
  } else {
    return ((*node1)->char_val - (*node2)->char_val);
  }
}

/* compares two huffnoes by ascii val, returns lesser */
int comp2(const void *a, const void *b) {
  huffnode **node1 = (huffnode **)a;
  huffnode **node2 = (huffnode **)b;

  return ((*node1)->char_val - (*node2)->char_val);
}

/* Recursively creates codes for each nodes in the tree,
   left is 0, right is 1 */
void create_codes(huffnode *node, char codes[], int i) {
  char *code;
  int j;
  if (node != NULL) {
    if (node->left == NULL && node->right == NULL) {
      codes[i] = '\0';
      code = malloc(i + 1);
      if (code == NULL) {
	perror("Code malloc failed");
	exit(3);
      }
      for (j = 0; j < (i + 1); j++) {
	code[j] = codes[j];
      }
      node->code = code;
    }
    if (node->left != NULL) {
      codes[i] = '0';
      create_codes(node->left, codes, i + 1);
    }
    if (node->right != NULL) {
      codes[i] = '1';
      create_codes(node->right, codes, i + 1);
    }
  }
}

char **codelist(huffnode **nodelist) {
  int size = nodelist[0]->size;
  int i;
  char **codelist = calloc(ASCIIVALS, sizeof(char *));
  if (codelist == NULL) {
    perror("Codelist calloc failed");
    exit(3);
  }
  for (i = 0; i < size; i++) {
    codelist[nodelist[i]->char_val] = nodelist[i]->code;
  }
  return codelist;
}

/* builds huffman tree, returns head of tree */
huffnode *build_tree(huffnode **nodelist, int size) {
  huffnode *huffll;
  huffnode *tempnode;
  int i = size;
  huffll = nodelist[0];
  while (i > 1) {
    tempnode = combine(huffll, huffll->next);
    huffll = huffll->next->next;
    /* insert into huffll */
    huffll = insert(huffll, tempnode);
    i--;
  }
  return huffll;
}
