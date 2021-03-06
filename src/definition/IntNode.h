#pragma once

#include "definition_global.h"

#include "DefinitionNode.h"

namespace paysages {
namespace definition {

/**
 * Node with a single integer value, for the definition tree.
 */
class DEFINITIONSHARED_EXPORT IntNode : public DefinitionNode {
  public:
    IntNode(DefinitionNode *parent, const string &name, int value = 0);

    inline int getValue() const {
        return value;
    }

    virtual string toString(int indent) const override;
    virtual void save(PackStream *stream) const override;
    virtual void load(PackStream *stream) override;
    virtual void copy(DefinitionNode *destination) const override;

    /**
     * Change the int value stored.
     *
     * The DiffManager is used as intermediary, so that the change may not happen immediately.
     */
    void setValue(int new_value);
    const IntDiff *produceDiff(int new_value) const;
    virtual void generateInitDiffs(vector<const DefinitionDiff *> *diffs) const override;
    virtual bool applyDiff(const DefinitionDiff *diff, bool backward = false) override;

  private:
    int value;
};
}
}
