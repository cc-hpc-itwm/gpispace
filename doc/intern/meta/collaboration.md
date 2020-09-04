# Collaboration

## Goals of this document

Describe tools and processes for collaboration in the GPI-Space team that allow for enjoyable, effective and efficient work.

## Terms
### GPI-Space:

Software framework to build parallel, distributed, scaling, robust frameworks and applications. GPI-Space is a collection of loosely coupled components.

### User:

A human or software that is a [system user](#system-user) or an [end-user](#end-user). The distinction is not always clear.

### System user:

A human or software that uses interfaces provided by [GPI-Space](#gpi-space) to implement a framework or an application to be used by [end user](#end-user)s. System users rely on documentation and templates provided by GPI-Space.

### End user:

A human or software that uses a framework or an application built on top of GPI-Space by [system user](#system-user)s. End users are not using interfaces provided by [GPI-Space](#gpi-space). End users might be unaware that the application or framework is built on top of GPI-Space. End users might be exposed to status and/or error messages produced by GPI-Space.

### Developer:

A human [implementer](#implementer) and/or [designer](#designer). The distinction is not always clear. Design questions will pop up during implementation and vice versa. However, it is important that developers are aware of their current role.

### Designer:

A human who defines the future software structure, it's interfaces and components. Designers take into account the current design and long term goals and milestones. Designers produce [definition](#definition)s to

- add wanted features and/or
- improve existing behavior and interfaces and/or
- improve software quality, robustness and maintainability.

### Definition:

Definitions consist of

- text documents and/or
- executable prototype code (in any language) and/or
- working change sets for [GPI-Space](#gpi-space)

and describes a reachable goal in sufficient details. Definitions clarify their relation with other definitions. Alternative definitions are discussed.

Definitions are either _proposed_ and might be _under discussion_ or _agreed_ and merged into the main line of the repository.

### Implementer:

A human who implements an [agreed definition](#definition). An implementation consists of tests, documentation and code. An implementation implements a single definition.

### What is _enjoyable_?
#### For [System user](#system-user)s

- Interfaces are easy to understand, robust and well documented.
- Interfaces are easy to use without surprises.
- Interfaces allow for a clear separatiom of the application logic.
- Frameworks and applications using [GPI-Space](#gpi-space) are easy to debug.
- Feedback from GPI-Space (status or error messages) is easy to understand and helpful.
- [End user](#end-user)s enjoy the frameworks or applications.

#### For [End user](#end-user)s

- Frameworks and applications do not leak raw information from [GPI-Space](#gpi-space).
- Frameworks and applications are robust, fast, reliable and scale well. They work for a wide range of inputs and provided resources.

#### For [Designer](#designer)s

- The direction of work is known, understood and accepted.
- [Definition](#definition)s are getting agreed.
- Definitions (or their implementations) are appreciated by users and/or developers and solve their problems.
- [Implementer](#implementer)s are able to and enjoy to implement definitions.

#### For [Implementer](#implementer)s

- Merge request are getting accepted.
- The direction of work and the context and scope of definitions is known, understood and accepted.
- New features are added.
- Bug are fixed.
- [User](#user)s enjoy [GPI-Space](#gpi-space) and or the frameworks and application built on top.
- Usage of build system is easy and does not consume attention. (Unless the implemented definition changes the build system.)
- Feedback from automatic builds is easy to understand and easy to reproduce.

### What is _effective_?

- Merge request are getting accepted.
- New features as requested by [user](#user)s are defined and implemented.
- Bugs are fixed.
- GPI-Space releases are published.

### What is _efficient_?

- Merge request are getting accepted without too many iterations of discussion and rebasing in reasonable short time.
- New features as requested by [user](#user)s are added in reasonable short time.
- Bugs are fixed in short time.
- Releases are published soon after features have been added and/or bugs have been fixed. In particular: No implemented features or bug fixes are left out in new releases.

## Tools
### Direct personal communication

Direct personal communication between [user](#user)s, [designer](#designer)s and [implementer](#implementer)s is a very important tool to exchange ideas, to discuss details and to synchronize views and estimations. Direct communication should always be considered.

#### Goals of a direct personal communication include:

- Exchange thoughts, ideas, impressions and feelings.
- Synchronize points of view.
- Discuss fuzzy details that are cumbersome to write down.

#### Non-Goals of a direct personal communication include:

- Produce formal [definition](#definition)s.
- Define strategies.

### Strategy workshop

A longer workshop to discuss strategies and directions of work. Workshops can happen as one day meeting at the institute or up to three days in form of a retreat. The horizon of a strategy workshop is 6 to 9 month. A strategy workshop requires a detailed agenda. The complete team attends a strategy workshop. Maybe [user](#user)s are invited to give input. Other potential attendees from the department include "bosses" and "managers of projects that uses [GPI-Space](#gpispace)".

#### Goals of a strategy workshop include:

- reflect the current state of the collaboration and the work process
- identify strong and weak points in the collaboration and the work process
- come up with ideas and actions to improve in the collaboration and the work process
- reflect the current state of [GPI-Space](#gpi-space)
- identify strong and weak points in GPI-Space
- come up with ideas and actions to improve GPI-Space

#### Non-goals of a strategy workshop include:

- Produce [definition](#definition)s.
- Discuss details.
- Discuss implementations.

### Design meeting

A short (<= 1h) meeting at the institute to discuss a single [proposed definition](#definition). The [designer](#designer) of the proposed definition is responsible to set up the meeting and to define an agenda. Typically the agenda consists of a short presentation of the definition by the designer (5-15 min) and time to ask questions and discuss alternative. The scope of the proposed definition is not left.

A design meeting might be attended by only a subset of the team members although in a small team it is often best to let the complete team discuss designs.

#### Goals of a design meeting include:

- Make developers aware of the scope and approach of the proposed definition.
- Present relations of the definition with other definitions to the developers.
- Make the designer aware of the questions, concerns, comments of the developers.
- Provide enough feedback to allow the designer to improve the proposed definition in order to agree on it finally.
- Provide enough information to allow the developers to make comments to the definition and propose alternatives in the corresponding [merge request](#merge-rquest-rfc).
- Identify situations when the proposed definition should be split, extended or discarded.

#### Non-Goals of a design meeting include:

- Discuss relation of the definition with other definitions.
- Discuss other definitions.
- Discuss possible implementations of the definition.

### (Bi-)Daily stand up

High frequency short meeting to mutually update all team members about the current status and the short term work plan. The horizon is one or two days. Time is not more than 5 minutes per team member.

#### Goals of a daily standup include:

- Distribute external news in the team. (urgent bugs, new users or projects, ...)
- Make team members mutually aware of the current status and short term work plan.
- Identify situations where [direct personal communication](#direct-personal-communition) is required.
- Identify situations where a [design meeting](#design-meeting) is required.
- Detect and synchronize short term conflicts.

#### Non-Goals of a daily standup include:

- Discuss [definition](#definition)s.
- Discuss implementations.

### Merge request: Definition

The request to agree on a proposed [definition](#definition). It is recommended that at least one iteration of self-review has been executed. In general the complete definition should become part of the repository main line. This might be hard or impossible if the definition includes change sets for GPI-Space. In this case the definition can be merged into a branch that clearly states that the definition is agreed. Also it is recommended that in this case the implementation of the definition starts immediately in order to avoid the effort to rebase the definition's branch too often.

#### Goals of a merge request for a Definition include:

- Agree on a definition and make it part of the maintained subset of the repository.
- Provide input for [design meeting](#design-meeting)s.
- Allow for an implementation of the definition.

#### Non-Goals of a merge request for a Definition include:

- Implement a definition.

### Merge request: Implementation

The request to merge a change set into the repository main line. A single definition is implemented in the change set. There are no "drive-by" changes whatsoever. Coding rules are respected. All automatic steps of verification and testing have been passed. It is recommended that at least two iterations of self-review have been executed.

#### Goals of a merge request for an implementation include:

- Implement a definition or fix a bug.
- Be self-contained and minimal.
- Be correct formally, e.g. there is a description, there are tests, there is documentation, there is an entry in the CHANGELOG.
- Respect coding rules or give reasons why not.

#### Non-Goals of a merge request for an implementation include:

- Implement more than a single definition.

### Code review: Definition

After a [proposed definition](#definition) has been presented in a [design meeting](#design-meeting) there will be a period of time (e.g. two weeks) to comment, ask question, give hints, propose improvements or approve a corresponding [merge request](#merge-request-definition) using the shared working environment. The common goal of designer and reviewer is to bring the definition to a point where it is agreed.

#### Goals of a code review of a definition include:

- Identify the situation where a proposed definition needs to be split, extended or discarded.
- Identify the situation where a (second, third, ...) design meeting is required.
- Approve the definition or help to improve in order to approve.

#### Non-Goals of a code review of a definition include:

- Discuss different definitions.
- Question the goal of the definition.
- Extend or change the scope of the definition unless the current scope does not reach the called goal.

### Code review: Implementation

A period of time (e.g. two weeks) to comment, ask question, give hints, propose improvements or approve a corresponding [merge request](#merge-request-implementation) using the shared working environment. The common goal of implementer and reviewer is to merge the change set into the main line of the repository.

#### Goals of a code review of an implementation include:

- Verify that the implementation appropriately implements the definition
- Identify the situation when the implementation is not appropriate (e.g. too general or too specific)
- Identify and mark bugs
- Identify and mark quirks
- Approve or recommend steps to improve the implementation

#### Non-Goals of a code review of an implementation include:

- Discuss the definition or alternative definitions


## Processes
### Record a bug

A bug gets reported by either a user or a developer by any kind of alien channel like chat, email, personal communication and so on. The bug report is imported into the official bug stream at gitlab and gets a number to become _recorded_. Bugs reports should contain enough information to enable developers to reproduce them. Please ask for clarification early.

### Resolve a bug

In order to resolve a recorded bug, it gets _assigned_ to one or more developers which are responsible to _understand_, _reproduce_, write a _test case_ and think about a _solution_. Depending on the scope and impact of the potential solution the next step might be a direct implementation for smaller bug or a design step for larger bugs. The assignments might change during the process.

Sometimes automated tests are not reasonable, for example when the bug is in the build system. In such cases the resolution of the bug must include a demonstration that the reproduced bug has been fixed.

### Trivial or cosmetic bug fix or improvement

A bug fix or improvement is trivial if it's implementation consists of a structural very small change set and take less then 15 minutes to implement. Examples include:

- fixes for typos in documentation or messages,
- improvements in layouts of documentation or messages, e.g. add missing line breaks
- typos in variable names,
- off by one errors.

A trivial change can be pushed directly into the main line of the repository. No issue is required, no merge request is required.

Note: White space changes are _not_ trivial.

Note: To be _structural_ small does not imply a small number of hunks. To fix the same typo at 20 different places is still a trivial change.

Note: In doubt opt for not pushing to master but open an issue and a merge request.

### Design a new feature

The team jointly assigns a certain amount of time to developers to design a new feature. The designer's goal is to produce and propose a definition in a design meeting and a subsequent merge request that gets approved and merged into the main line of the repository. Designers take great care to clearly

- motivate the new feature, e.g. what are the benefits for the users,
- describe the new feature in detail,
- discuss existing solutions in GPI-Space and outside of GPI-Space and their pros and cons,
- discuss alternative solutions in GPI-Space and maybe sketch their interfaces,
- draw a conclusion about the aimed solution,
- describe the interfaces of the aimed solution,
- discuss how the described interfaces can be used to get the motivated benefits and
- discuss which parts of GPI-Space needs to be changed or extended.

Designers freely choose the tools that they think are best suited produce a definition that gets accepted, merged and implemented.

Designers rely on the results of strategy workshop and use direct personal communication as first and frequent choice to ensure early exchange and information flow across the whole team.

If the definition can not be produced within the assigned amount of time, then the team might jointly decide to prolong the design phase. A design phase must not be extended more than two times. If a design can not be produced after two prolongations, then it is stopped and put onto a list of topics to be discussed in a strategy workshop.

### Implement a new feature

The team jointly assigns a certain amount of time to developers to implement a new feature or a bug fix. The implementers goal is to produce a merge request for the implementation that gets approved and merged into the main line of the repository.

While an implementation is typically based on a definition, there might be cases where no definition exists. In any case, the first step of the implementation is to understand the goal of the new feature or the scope of the bug fix in detail and to derive an _implementation plan_. The implementation plan consists of a list of changed or extended components and the scope and complexity of each individual change or extension. The implementation plan is short enough to be presented in a (bi-)daily stand up meeting and detailed enough to enable team members to understand the required effort.

The implementation plan is not meant to become part of the merge request or the repository and not every implementation plan needs to be presented in the (bi-)daily stand up. However, for each implementation there should be a _written_ implementation plan. The write up might be a text file, some notes on a sheet of paper or some white board drawing. The crucial point is: It shall exist and implementers must be able to present it upon request.

Implementers take great care to

- respect all coding guide lines and formal rules,
- continuously update the implementation plan during the implementation phase and feed back relevant changes into the (bi-)daily stand up immediately,
- make sure all scenarios described in the definition are tested,
- make sure known corner cases and degenerated cases are tested,
- make sure all user inputs are verified,
- make sure all messages clearly state a reason or purpose and reflect users input,
- produce code that is easy to maintain,
- produce code that is easy to maintain,
- produce code that is easy to maintain,
- produce effective code,
- produce code without surprises and
- produce efficient code.

If an implementation has not been finished within the assigned amount of time, then the team might jointly decide to prolong the implementation phase or to cut the implementation into smaller pieces. In both cases the implementation plan must be updated. It might be required to stop the implementation and start a new design phase. After at most two prolongations the implementation is stopped and the topic of the definition is put to a list of topics to be discussed in a strategy workshop.

### Do a code review

The team jointly assigns a certain amount of time to developers to do a code review for a proposed definition or implementation. The reviewers goal is to verify that the merge request is formally correct and fulfills the called goals.

Each reviewer first reads the complete merge request at least once and then writes a summary that reflects the findings, does an assessment and either approves the merge request or gives recommendations how to improve the merge request in order to get approved. There might be the case that reviewers reject merge request in which case all reviewers immediately stop working on the merge request after they wrote their summary and the merge request is closed. The topic is put onto a list of topics to be discussed in the next strategy meeting. Note: Rejection most probably indicates a failure of management!

In a second pass reviewers add comments, ask questions, give hints or propose improvements. It is important to first write the summary in order to refer to it and in order to first put the authors into context.

Reviewers followup within the assigned amount of time and elaborate, resolve discussions and take part in the discussion in a timely manner, at least once every three working days.

If all reviewers have approved a merge request, then it will be merged and open discussions are resolved into follow up issues. If a merge request has not been approved by all reviewers within the assigned amount of time and there are no open discussions, then it will be merged. If a merge request has not been approved by all reviewers within the assigned amount of time and there are open discussions, then the team might jointly decide to prolong the review phase. If a merge request can not be merged after two prolongations, then it is closed and its topic is put to list of topics to be discussed in a strategy meeting.

### Get a code review

To get a code review is about the project. It is not about the developer.

Developers ensure to answer all questions from reviewers in a timely manner, at least after two working days. They take into account hints and proposed improvements and either update their merge request accordingly, or open issues to delegate the work into the future or explain why the hints or proposed improvements do not apply. Developers might close discussions only after they have been updated the merge request and in this case they mention the commit that closes the discussion. In doubt they just mention the update in the discussion and wait for the reviewer who opened the discussion to close it.
