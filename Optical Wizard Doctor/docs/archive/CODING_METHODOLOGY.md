# Coding & Prompting Methodology - Optical Wizard Doctor Project

**Date:** December 2024  
**Project:** Optical Wizard Doctor (OWD) - C++/Qt6 Desktop Application  
**Methodology:** "Vibe Coding" - Structured, Risk-Aware, Incremental Development

---

## Executive Summary

This document captures the coding and prompting methodology used throughout the Optical Wizard Doctor project. This approach emphasizes **preemptive problem-solving**, **proven methods**, **incremental testing**, and **deep understanding** before implementation. The methodology has proven highly effective for building complex, production-ready software with minimal debugging cycles.

---

## Core Philosophy

### 1. **Preemptive Problem-Solving Over Reactive Debugging**

**Principle:** Identify and solve problems *before* they occur, not after.

**Practice:**
- Conduct comprehensive risk analysis before starting any phase
- Research proven methods and patterns before implementation
- Document potential issues and mitigation strategies upfront
- Predict obstacles and prepare solutions in advance

**Example:**
```
Before Phase 2: "analyze it and predict anything that could go wrong, be an obstacle, or be difficult to set up or do right. i want you to know what youre getting into and not do problem solving on the fly."
```

### 2. **Proven Methods Only - No Custom Solutions**

**Principle:** Use established, documented patterns. Avoid inventing new solutions.

**Practice:**
- Research Stack Overflow for every implementation pattern
- Cite sources for all methods used
- Verify methods are industry-standard before using
- Document why chosen method is better than alternatives

**Example:**
```
"do some bulk internet research and find proven methods to implement. and document those/specify how we'll use them before we do it."
"find out if theres anything that requires a non proven method that you have to 'concoct'."
"cite something from Stack overflow for all of the items in that"
```

### 3. **Understanding "Why" Before "What"**

**Principle:** Understand the rationale, trade-offs, and alternatives before implementing.

**Practice:**
- Discuss why a particular approach was chosen
- Explore alternative approaches and their pros/cons
- Consider computational cost and efficiency
- Evaluate if the solution is the most efficient approach

**Example:**
```
"why dont we talk about why you chose to build it that way? are there other ways to build it? is that the most efficient solution? is it expensive computationally? these are questions we need to ask before we finish each phase. why are we doing it this way? and not another way? what are the other ways to do it? and why did we choose this one over those ones? what are the pros and cons to the other ways?"
```

### 4. **Sequence and Dependency Enforcement**

**Principle:** Strict adherence to implementation sequence. No shortcuts.

**Practice:**
- Never proceed until prerequisites are met
- Complete phases fully before moving to next
- Test thoroughly at each checkpoint
- Verify dependencies are satisfied before starting work

**Example:**
```
"always test, and ensure we're using the right stuff first before moving on to the next steps"
"look forward in the sequence and predict what issues we'll run into and prepare for them"
```

---

## The Methodology: Step-by-Step Process

### Phase 0: Pre-Implementation (Before Any Code)

#### Step 1: Risk Analysis
**Purpose:** Identify all potential problems, obstacles, and risks before coding begins.

**Process:**
1. Analyze the phase requirements
2. Identify critical dependencies and integration challenges
3. List potential issues (coordinate system confusion, data loss, file I/O errors, etc.)
4. Categorize risks by severity (Critical, High, Medium, Low)
5. Document solutions and mitigation strategies for each risk
6. Create prevention checklists

**Deliverable:** Comprehensive risk analysis document (e.g., `PHASE_X_RISK_ANALYSIS.md`)

**Example Structure:**
```markdown
### Risk 1: [Risk Name]
- **Problem:** Description of the problem
- **Potential Issues:** List of specific issues
- **Solutions:** Detailed solutions
- **Prevention Checklist:** Action items
```

#### Step 2: Proven Methods Research
**Purpose:** Research and document established implementation patterns.

**Process:**
1. Research Stack Overflow for implementation patterns
2. Review Qt documentation and best practices
3. Identify industry-standard approaches
4. Document each method with:
   - Standard pattern/code example
   - Stack Overflow citations
   - Qt documentation references
   - How we'll use it
   - Status (Proven vs Custom)
5. Verify no custom solutions are required

**Deliverable:** Proven methods document (e.g., `PHASE_X_PROVEN_METHODS.md`)

**Key Requirement:** Every method must have Stack Overflow citations.

#### Step 3: Architecture Discussion
**Purpose:** Understand design decisions before implementation.

**Process:**
1. Present proposed implementation approach
2. Discuss alternatives and trade-offs
3. Evaluate computational cost
4. Consider efficiency and maintainability
5. Get approval before proceeding

**Question Framework:**
- Why this approach?
- What are the alternatives?
- What are the pros/cons of each?
- Is this the most efficient solution?
- What are the computational costs?

### Phase 1: Incremental Implementation

#### Step 1: Small, Testable Units
**Principle:** Break work into small, testable increments.

**Practice:**
- Implement one feature at a time
- Test thoroughly before moving on
- Back-check for regressions
- Don't batch process when asked to do manually

**Example:**
```
"ok now you can begin with phase two in incremental steps, testing for functionality thoroughly as we progress. back checking too to ensure that our new additions and features dont mess up things we did before."
```

#### Step 2: Build and Verify
**Practice:**
- Always build after every change
- Never deliver without building
- Fix compilation errors immediately
- Run linting and type checking

**Rule:**
```
"always build after every typescript/next js change, never tell me you made a change without first building and linting and typechecking before delivering"
```

#### Step 3: Provide Test Checklist
**Practice:**
- Create comprehensive test checklist for each feature
- Include basic functionality, edge cases, integration, and regression tests
- Track logs during testing
- User provides feedback, agent reads logs and fixes issues

**Example Workflow:**
1. Agent: "Here's the test checklist for Feature X"
2. User: Tests and provides feedback
3. Agent: Reads logs, identifies issues
4. Agent: Fixes issues
5. Repeat until feature is complete

### Phase 2: Deep Debugging When Issues Arise

#### Step 1: Add Comprehensive Logging
**Practice:**
- When functionality doesn't work, add detailed logging
- Track what happens at each step
- Log state changes, function calls, and data flow
- Use `qDebug()` for Qt applications

**Example:**
```
"can you do a deep dive into debugging it? maybe add logging and track what happens when i try to do it..."
```

#### Step 2: Analyze Logs Systematically
**Practice:**
- Read logs carefully
- Identify where the flow breaks
- Trace the execution path
- Compare expected vs actual behavior

**Example:**
```
"you can check the logs, i tried resizing and moving, but nothing happened, can you run it one more time and track the logs again?"
```

#### Step 3: Fix Root Cause, Not Symptoms
**Practice:**
- Identify the root cause, not just the symptom
- Fix the underlying issue
- Verify the fix doesn't break other things
- Test thoroughly after fix

### Phase 3: User Testing Workflow

#### The Testing Cycle

1. **Agent Builds and Provides Checklist**
   - Agent builds the application
   - Agent creates test checklist
   - Agent tracks logs

2. **User Tests and Provides Feedback**
   - User tests the feature
   - User reports what happened
   - User reports what they saw

3. **Agent Analyzes and Fixes**
   - Agent reads logs
   - Agent identifies issues
   - Agent fixes issues
   - Agent rebuilds

4. **Repeat Until Complete**
   - Continue cycle until all tests pass
   - Verify no regressions
   - Move to next feature

**Example:**
```
"run it and give me a list of actions to test and track the logs, and then ill tell you what happened and what i saw, and you can read the logs and we'll address any issues head on right now."
```

---

## Key Techniques and Patterns

### 1. Risk-First Development

**Pattern:** Always start with risk analysis, never jump to code.

**Why:** Prevents coding into corners, identifies problems early, saves debugging time.

**When:** Before every major phase or feature.

### 2. Proven Methods Research

**Pattern:** Research and cite Stack Overflow for every implementation pattern.

**Why:** Ensures using battle-tested solutions, avoids reinventing the wheel, provides documentation.

**When:** Before implementing any new feature or pattern.

### 3. Incremental Testing

**Pattern:** Test thoroughly after each small change, don't batch test.

**Why:** Catches issues early, easier to debug, prevents cascading failures.

**When:** After every feature implementation.

### 4. Back-Checking

**Pattern:** Verify new features don't break existing functionality.

**Why:** Prevents regressions, maintains stability, ensures quality.

**When:** After every new feature or change.

### 5. Deep Logging for Debugging

**Pattern:** Add comprehensive logging when issues arise, track execution flow.

**Why:** Makes debugging systematic, reveals hidden issues, provides visibility.

**When:** When functionality doesn't work as expected.

### 6. Architecture Discussion

**Pattern:** Discuss design decisions, alternatives, and trade-offs before implementation.

**Why:** Ensures optimal solutions, prevents rework, builds understanding.

**When:** Before implementing major features or architectural changes.

### 7. User-Driven Testing

**Pattern:** User tests with real scenarios, agent analyzes logs and fixes.

**Why:** Real-world testing, catches edge cases, ensures usability.

**When:** After every feature implementation.

### 8. Sequence Enforcement

**Pattern:** Strict adherence to dependency order, no parallel work on dependencies.

**Why:** Prevents integration issues, ensures stability, maintains quality.

**When:** Always, throughout the project.

---

## Documentation Standards

### Required Documents for Each Phase

1. **Risk Analysis Document** (`PHASE_X_RISK_ANALYSIS.md`)
   - All identified risks
   - Solutions and mitigations
   - Prevention checklists

2. **Proven Methods Document** (`PHASE_X_PROVEN_METHODS.md`)
   - All implementation patterns
   - Stack Overflow citations
   - Code examples
   - Usage instructions

3. **Progress Reports** (as needed)
   - What was completed
   - Issues encountered
   - Solutions applied

4. **Test Checklists** (for each feature)
   - Basic functionality tests
   - Edge case tests
   - Integration tests
   - Regression tests

### Code Documentation

- Inline comments for complex logic
- Function documentation (Doxygen style)
- Architecture decisions documented
- Known issues and TODOs tracked

---

## Quality Assurance Practices

### 1. Build Verification
- Always build after changes
- Fix compilation errors immediately
- Run linting
- Type checking (where applicable)

### 2. Code Review
- Review code for issues before delivery
- Check for common mistakes
- Verify patterns are followed
- Ensure consistency

### 3. Testing
- Comprehensive test checklists
- User-driven testing
- Regression testing
- Edge case testing

### 4. Logging and Debugging
- Add logging when issues arise
- Track execution flow
- Analyze logs systematically
- Fix root causes

### 5. Codacy Analysis
- Run Codacy analysis after file edits
- Fix identified issues
- Maintain code quality standards

---

## Communication Patterns

### 1. Direct and Concise
- Be direct and to the point
- Avoid unnecessary verbosity
- Focus on actionable information

### 2. Code References
- Use `startLine:endLine:filepath` format for existing code
- Show code context when explaining
- Reference specific files and functions

### 3. Progress Updates
- Update TODOs as work progresses
- Mark tasks complete immediately
- Track what's in progress

### 4. Question Framework
- Ask for clarification when unsure
- Don't assume or guess
- Verify understanding before proceeding

---

## Anti-Patterns (What NOT to Do)

### 1. ❌ Jumping to Code Without Analysis
**Why Bad:** Leads to problems discovered late, requires rework, wastes time.

**Correct Approach:** Risk analysis → Proven methods → Architecture discussion → Implementation

### 2. ❌ Custom Solutions Without Research
**Why Bad:** Reinvents the wheel, introduces bugs, lacks documentation.

**Correct Approach:** Research proven methods, cite sources, use established patterns.

### 3. ❌ Batch Processing When Asked to Do Manually
**Why Bad:** Violates user instructions, can cause issues, reduces control.

**Correct Approach:** Process one by one as requested.

### 4. ❌ Skipping Tests
**Why Bad:** Issues discovered late, harder to debug, reduces quality.

**Correct Approach:** Test thoroughly after each change, provide checklists.

### 5. ❌ Assuming Data Contents
**Why Bad:** Can lead to incorrect logic, bugs, data corruption.

**Correct Approach:** Manually check data, verify it's meaningful, admit uncertainty.

### 6. ❌ One-Size-Fits-All Solutions
**Why Bad:** Doesn't respect nuances, can cause issues, reduces flexibility.

**Correct Approach:** Approach each category independently, respect specific nuances.

### 7. ❌ Starting Server Without Permission
**Why Bad:** Violates user rules, can cause conflicts, wastes resources.

**Correct Approach:** Never start server unless explicitly instructed.

---

## Success Metrics

### What Made This Methodology Successful

1. **Minimal Debugging Cycles**
   - Preemptive problem-solving caught issues early
   - Risk analysis prevented major problems
   - Proven methods reduced bugs

2. **High Code Quality**
   - Incremental testing caught issues early
   - Back-checking prevented regressions
   - Code review maintained standards

3. **Efficient Development**
   - Clear sequence prevented rework
   - Proven methods saved research time
   - User testing caught real-world issues

4. **Comprehensive Documentation**
   - Risk analyses documented decisions
   - Proven methods provided references
   - Handoff documents enabled continuity

5. **User Satisfaction**
   - Features worked as expected
   - Issues were fixed quickly
   - Quality was maintained throughout

---

## Lessons Learned

### What Worked Well

1. **Risk Analysis Before Implementation**
   - Identified problems early
   - Prevented coding into corners
   - Saved debugging time

2. **Proven Methods Research**
   - Used battle-tested solutions
   - Avoided reinventing the wheel
   - Provided documentation

3. **Incremental Testing**
   - Caught issues early
   - Easier to debug
   - Maintained quality

4. **User-Driven Testing**
   - Real-world scenarios
   - Caught edge cases
   - Ensured usability

5. **Deep Logging for Debugging**
   - Made debugging systematic
   - Revealed hidden issues
   - Provided visibility

### What Could Be Improved

1. **Earlier Code Review**
   - Could catch issues before user testing
   - Would reduce iteration cycles

2. **Automated Testing**
   - Could catch regressions automatically
   - Would reduce manual testing burden

3. **Performance Profiling**
   - Could identify bottlenecks earlier
   - Would optimize proactively

---

## Application to Other Projects

### When to Use This Methodology

**Best For:**
- Complex projects with multiple phases
- Projects requiring high reliability
- Projects with strict dependencies
- Projects needing comprehensive documentation
- Projects where quality is critical

**May Be Overkill For:**
- Simple, single-feature projects
- Rapid prototypes
- Projects with loose requirements
- Projects with flexible timelines

### Adapting the Methodology

**For Smaller Projects:**
- Condense risk analysis (still do it, but shorter)
- Reduce documentation (focus on essentials)
- Maintain testing rigor
- Keep proven methods research

**For Larger Projects:**
- Expand risk analysis (more detail)
- Increase documentation (more comprehensive)
- Add automated testing
- Implement continuous integration

**For Different Domains:**
- Adapt risk categories to domain
- Research domain-specific patterns
- Adjust testing approach to domain
- Maintain core principles

---

## Conclusion

This methodology emphasizes **preemptive problem-solving**, **proven methods**, **incremental testing**, and **deep understanding** before implementation. It has proven highly effective for building complex, production-ready software with minimal debugging cycles.

**Key Takeaways:**
1. Always analyze risks before coding
2. Research proven methods and cite sources
3. Test incrementally and thoroughly
4. Understand "why" before "what"
5. Maintain strict sequence and dependency enforcement
6. Use deep logging for systematic debugging
7. Document decisions and patterns
8. Never skip quality assurance steps

**The methodology is not just about coding—it's about thinking, planning, and understanding before doing.**

---

## Appendix: Quick Reference Checklist

### Before Starting Any Phase
- [ ] Risk analysis completed
- [ ] Proven methods researched and documented
- [ ] Stack Overflow citations obtained
- [ ] Architecture discussed and approved
- [ ] Dependencies verified

### During Implementation
- [ ] Incremental, testable units
- [ ] Build after every change
- [ ] Test thoroughly before moving on
- [ ] Back-check for regressions
- [ ] Update TODOs

### When Issues Arise
- [ ] Add comprehensive logging
- [ ] Track execution flow
- [ ] Analyze logs systematically
- [ ] Fix root cause, not symptoms
- [ ] Test fix thoroughly

### After Implementation
- [ ] Provide test checklist
- [ ] User tests and provides feedback
- [ ] Analyze logs and fix issues
- [ ] Verify no regressions
- [ ] Document decisions

---

**This methodology is a living document. Update it as you learn and improve.**


